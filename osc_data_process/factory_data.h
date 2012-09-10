#pragma once

#include <vector>

#include "base/memory/ref_counted.h"

// TODO need function to give FacotryData the already new char array. 
// no copy again.
class FactoryData 
    : public base::RefCountedThreadSafe<FactoryData> {
public:
  FactoryData();
  virtual ~FactoryData() {};

  const std::vector<char>& data() const {
    return data_;
  }
  void set_data(std::vector<char>& data) {
    data_ = data;
  }
  int id() const {
    return id_;
  }
  void set_id(int id) {
    id_ = id;
  }

 
private:
  std::vector<char> data_;
  int id_;
};
