{
  "targets": [
    {
      "target_name": "levenshtein",
      "sources": ["damerau-levenshtein.cc"],
      "conditions": [
        [
          "OS==\"mac\"",
          {
            "cflags_cc": ["-fno-exceptions", "-O2"],
            "cflags": ["-std=c++11", "-g", "-O2"],
            "xcode_settings": {
              "GCC_ENABLE_CPP_EXCEPTIONS": "YES"
            }
          }
        ],
        [
          "OS==\"linux\"",
          {
            "cflags_cc": ["-fno-exceptions", "-O2"],
            "cflags": ["-std=c++11", "-g", "-O2"]
          }
        ]
      ]
    }
  ]
}
