{
  "targets": [
    {
      "target_name": "julia",
      "sources": [ "src/j2.cpp", "src/embedded.cpp" ],
      "include_dirs": ["include"],
        'libraries': ['<!@(/Applications/Julia-0.5.app/Contents/Resources/julia/share/julia/julia-config.jl --ldlibs)'],
    'cflags': ['-std=c++14'],
      "xcode_settings": {
            'OTHER_CFLAGS': ['-std=c++14', '<!@(/Applications/Julia-0.5.app/Contents/Resources/julia/share/julia/julia-config.jl --cflags)'],
'OTHER_LDFLAGS': ['<!@(/Applications/Julia-0.5.app/Contents/Resources/julia/share/julia/julia-config.jl --ldflags)'],
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
