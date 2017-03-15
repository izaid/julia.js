{
    'variables' : {'julia_config': '<!(julia -e "println(joinpath(dirname(JULIA_HOME), \\"share\\", \\"julia\\", \\"julia-config.jl\\"))")'},
  "targets": [
    {
      "target_name": "julia",
      "sources": [ "src/j2.cpp", "src/embedded.cpp" ],
      "include_dirs": ["include"],
        'libraries': ['<!@(<(julia_config) --ldlibs)'],
    'cflags': ['-std=c++14'],
      "xcode_settings": {
            'OTHER_CFLAGS': ['-std=c++14', '<!@(<(julia_config) --cflags)'],
'OTHER_LDFLAGS': ['<!@(<(julia_config) --ldflags)'],
      }
    },
    {
         "target_name": "copy_binary",
         "type":"none",
         "dependencies" : [ "julia" ],
         "copies":
         [
            {
               'destination': '<(module_root_dir)',
               'files': ['<(module_root_dir)/build/Release/julia.node']
            }
         ]
      }
  ]
}
