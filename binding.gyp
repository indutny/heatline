{
  "targets": [
    {
      "target_name": "heatline",

      "include_dirs": [
        "src",
        "<!(node -e \"require('nan')\")",
      ],

      "sources": [
        "src/heatline.cc",
      ],
    }
  ]
}
