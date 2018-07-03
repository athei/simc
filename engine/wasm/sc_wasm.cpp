// ==========================================================================
// Dedmonwakeen's Raid DPS/TPS Simulator.
// Send questions to natehieter@gmail.com
// ==========================================================================

#include "simulationcraft.hpp"
#include "util/git_info.hpp"

#include <emscripten.h>
#include <fstream>
#include <cstdio>
#include <chrono>

extern "C"
{

void js_loaded();

void js_send_progress(uint32_t iteration,
                      uint32_t total_iterations,
                      uint32_t phase,
                      uint32_t total_phases,
                      const char* phase_name,
                      const char* subphase_name);

}

namespace wasm_hooks
{

void update_progress( const sim_progress_t &progress,
                      size_t current_phase,
                      size_t total_phases,
                      const std::string &phase_name,
                      const std::string &subphase_name )
{
  using progress_clock = std::chrono::steady_clock;
  static const auto progress_interval = std::chrono::seconds{1};
  static auto last = progress_clock::time_point{progress_clock::duration::zero()};
  const auto now = progress_clock::now();

  if ((now - last) < progress_interval)
    return;

  js_send_progress(progress.current_iterations, progress.total_iterations, current_phase,
                   total_phases, phase_name.c_str(), subphase_name.c_str());
  last = now;
}

}

namespace
{

void initialize_data()
{
  dbc::init();
  module_t::init();
  unique_gear::register_hotfixes();
  unique_gear::register_special_effects();
  unique_gear::sort_special_effects();
  hotfix::apply();
}

const uint8_t *cpp_simulate( const std::string& profile )
{
  sim_t simc;
  sim_control_t control;

  control.options.parse_text( profile );
  simc.setup( &control );
  simc.json_file_str = "/output.json";
  simc.report_progress = 1;

  util::printf(
      "\nSimulating... ( iterations=%d, threads=%d, target_error=%.3f,  max_time=%.0f, vary_combat_length=%0.2f, "
      "optimal_raid=%d, fight_style=%s )\n\n",
      simc.iterations, simc.threads, simc.target_error, simc.max_time.total_seconds(), simc.vary_combat_length,
      simc.optimal_raid, simc.fight_style.c_str() );

  if ( !simc.execute() )
  {
    util::printf( "Simulation was canceled.\n" );
    return NULL;
  }

  simc.scaling->analyze();
  simc.plot->analyze();
  simc.reforge_plot->analyze();
  report::print_json( simc );

  // TODO: change simcraft to write json to RAM instead of file
  std::ifstream file{ "/output.json", std::ifstream::ate | std::ifstream::binary };
  const auto filesize = file.tellg();
  const auto string_size = filesize + static_cast<decltype(filesize)>(1u);
  file.seekg(0);
  uint8_t *buffer = static_cast<decltype(buffer)>(malloc(string_size));
  file.read( (char *)buffer, filesize );
  buffer[filesize] = 0; // null terminate
  std::remove( "/output.json" );

  return buffer;
}

}  // anonymous namespace ====================================================

// ==========================================================================
// WASM API (must be C compatible)
// ==========================================================================

extern "C"
{

EMSCRIPTEN_KEEPALIVE
int main( int, char** )
{
  initialize_data();
  js_loaded();
}

EMSCRIPTEN_KEEPALIVE
const uint8_t *simulate(const char* profile_str)
{
  try {
    return cpp_simulate(profile_str);
  }
  catch( const std::exception& e ){
    std::cerr << e.what() << std::endl;
    return NULL;
  }
}

}
