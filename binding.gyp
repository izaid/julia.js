{
  "targets": [
    {
      "target_name": "julia",
      "sources": [ "src/j2.cpp", "src/julia.cpp" ],
      "include_dirs": ["include"],
        'libraries': ['<!@(/Applications/Julia-0.5.app/Contents/Resources/julia/share/julia/julia-config.jl --ldlibs)'],
      "xcode_settings": {
            'OTHER_CFLAGS': ['<!@(/Applications/Julia-0.5.app/Contents/Resources/julia/share/julia/julia-config.jl --cflags)'],
'OTHER_LDFLAGS': ['<!@(/Applications/Julia-0.5.app/Contents/Resources/julia/share/julia/julia-config.jl --ldflags)'],
      }
    },
    {
      "target_name": "_v8",
      'type': 'static_library',
      "sources": [ "src/_v8.cpp", "src/j2.cpp" ],
      "include_dirs": ["include", "<!(node -e \"require('nan')\")"],
        'libraries': ['-lv8', '-lv8_libbase','-lv8_libplatform', '<!@(/Applications/Julia-0.5.app/Contents/Resources/julia/share/julia/julia-config.jl --ldlibs)'],
      "xcode_settings": {
          'MACOSX_DEPLOYMENT_TARGET': '10.12',
            'OTHER_CFLAGS': ['-fPIC', '<!@(/Applications/Julia-0.5.app/Contents/Resources/julia/share/julia/julia-config.jl --cflags)'],
'OTHER_LDFLAGS': ['<!@(/Applications/Julia-0.5.app/Contents/Resources/julia/share/julia/julia-config.jl --ldflags)'],
      }
    }
  ]
}
