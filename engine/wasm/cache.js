Module['instantiateWasm'] = function(imports, successCallback) {
  self.simcCallbacks.instantiateWasm(imports).then(function(instance) {
    successCallback(instance);
  });
  return {};
};
