{
    'variables' : {'julia_config': '<!(julia -e "println(joinpath(dirname(JULIA_HOME), \\"share\\", \\"julia\\", \\"julia-config.jl\\"))")'},
    "targets": [
        {
            "target_name": "julia",
            "sources": [ "src/j2.cpp", "src/embedded.cpp" ],
            "include_dirs": ["include"],
            'libraries': ['<!@(<(julia_config) --ldlibs)'],
            'cflags': ['<!@(<(julia_config) --cflags)', '-Werror -Wall -fsanitize=address'],
            'ldflags': ['<!@(<(julia_config) --ldflags)', '-fsanitize=address -lasan'],
            "xcode_settings": {
                'MACOSX_DEPLOYMENT_TARGET': '10.9',
                'OTHER_CFLAGS': [ '<!@(<(julia_config) --cflags)', '-Werror -Wall -fsanitize=address'],
                'OTHER_LDFLAGS': ['<!@(<(julia_config) --ldflags)', '-fsanitize=address -lasan'],
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
