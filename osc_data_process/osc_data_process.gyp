{
  'variables': {
    'chromium_code': 1,
  },
  
  'targets' : [
    {
      'target_name': 'osc_data_process',
      'type': 'executable',
      'sources': [
        'factory.h',
        'factory.cc',
        'factory_data.h',
        'factory_data.cc',
        'factory_host.h',
        'factory_host.cc',
        'main.cc',
        'observer_cross_thread.h',
        'observer_cross_thread.cc',
      ],
      'dependencies': [
        '../base/base.gyp:base',
      ],
    },

  ],
}
