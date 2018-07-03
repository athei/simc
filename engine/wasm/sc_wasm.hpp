#ifndef SC_WASM_HPP
#define SC_WASM_HPP

#include <simulationcraft.hpp>

namespace wasm_hooks
{
  void update_progress( const sim_progress_t &progress,
                        size_t current_phase,
                        size_t total_phases,
                        const std::string &phase_name,
                        const std::string &subphase_name );
}

#endif /* SC_WASM_HPP */
