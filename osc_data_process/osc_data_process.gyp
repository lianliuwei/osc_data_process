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
  {
      'target_name': 'osc_data_process_unittests',
      'type': '<(gtest_target_type)',
      'sources': [
	      'factory_unittest.cc',
        
        'factory.h',
        'factory.cc',
        'factory_data.h',
        'factory_data.cc',
        'factory_host.h',
        'factory_host.cc',
        'observer_cross_thread.h',
        'observer_cross_thread.cc',
      ],
      'dependencies': [
        '../base/base.gyp:base',
        '../base//base.gyp:test_support_base',
        #'third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '../testing/gmock.gyp:gmock',
        '../testing/gtest.gyp:gtest',        
      ],
    },
  ],
}
