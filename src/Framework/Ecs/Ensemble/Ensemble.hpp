#pragma once

#include <tuple>
#include <type_traits>



namespace ecs::ensemble {
template <template <typename... Args> typename container, typename... Args>
class Ensemble
{
  using Storage = std::tuple<container<Args>...>;

public:
  Ensemble()=default;

  template <typename... Ts,
            typename = std::enable_if_t<std::is_same_v<
                std::tuple<Ts...>, std::tuple<container<Args>...>>>>
  explicit Ensemble(Ts... args):ensemble_(args...)
  {
  }

  Ensemble(const Ensemble& other):ensemble_(other.ensemble_){}
  Ensemble(Ensemble&& other) noexcept :ensemble_(std::move(other.ensemble_)){
    
  }

  ~Ensemble()=default;

  template<typename T>
  T find()
  {}

private:
  Storage ensemble_;
};
}  // namespace ecs::ensemble