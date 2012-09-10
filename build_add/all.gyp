{
  'targets': [
    {
      'target_name': 'all',
      'type': 'none',
      'dependencies': [
        '../base/base.gyp:*',
        '../sample/base.gyp:*',
        '../osc_data_process/osc_data_process.gyp:*',
      ],
    }, # target_name: All
  ], # conditions
}