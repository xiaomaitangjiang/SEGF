#pragma once


#include <type_traits>

class basic_Typecast
{
public:
  virtual ~basic_Typecast() = default;
  virtual void data(void* element) = 0;
};

template <typename Original>
class Typecast : public basic_Typecast
{
  using type = std::remove_cv_t<Original>;

public:
  explicit Typecast()=default;

  type data(void* element) override
  {
    return static_cast<type>(element);
  }

};
