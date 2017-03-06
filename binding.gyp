{
  "targets": [
    {
      "target_name": "j2",
      "sources": [ "src/j2.cpp" ],
      "include_dirs": ["include"],
      "xcode_settings": {
            'OTHER_CFLAGS': ['<!@(/Applications/Julia-0.5.app/Contents/Resources/julia/share/julia/julia-config.jl --cflags)'],
      }
    }
  ]
}
