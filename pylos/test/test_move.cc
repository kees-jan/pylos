#include <array>
#include <bitset>
#include <cassert>
#include <gsl/gsl_assert>
#include <gsl/gsl_util>

#include <catch2/catch.hpp>

#define Bug() GSL_CONTRACT_CHECK("Bug", false)

namespace pylos
{
  constexpr uint32_t                               number_of_layers = 4;
  constexpr std::array<uint32_t, number_of_layers> number_of_balls_in_layer_{16, 9, 4, 1};

  [[nodiscard]] constexpr uint32_t number_of_balls_in_layer(uint32_t l) { return gsl::at(number_of_balls_in_layer_, l); }

  constexpr auto number_of_balls_in_lower_layers_ = [] {
    std::array<uint32_t, number_of_layers + 1> t{};
    uint32_t                                   sum{0};

    for(uint32_t i = 0; i < number_of_layers; i++)
    {
      gsl::at(t, i) = sum;
      sum += number_of_balls_in_layer(i);
    }
    t[number_of_layers] = sum;
    return t;
  }();

  [[nodiscard]] constexpr uint32_t number_of_balls_in_lower_layers(uint32_t l)
  {
    return gsl::at(number_of_balls_in_lower_layers_, l);
  }

  [[nodiscard]] constexpr uint32_t size_of_layer(uint32_t l)
  {
    Expects(l < number_of_layers);
    return number_of_layers - l;
  }

  class [[nodiscard]] integer_position
  {
  public:
    explicit constexpr integer_position(uint32_t i)
      : position(i)
    {
      Expects(i < 30);
    }

    [[nodiscard]] constexpr uint32_t x() const
    {
      return (position - number_of_balls_in_lower_layers(layer())) % size_of_layer(layer());
    }

    [[nodiscard]] constexpr uint32_t y() const
    {
      return (position - number_of_balls_in_lower_layers(layer())) / size_of_layer(layer());
    }

    [[nodiscard]] constexpr uint32_t layer() const
    {
      for(uint32_t l = 0; l < number_of_layers; l++)
      {
        if(number_of_balls_in_lower_layers(l + 1) > position)
        {
          return l;
        }
      }

      Bug();
    }

  private:
    uint32_t position;
  };

  class [[nodiscard]] coordinate_position
  {
  public:
    constexpr coordinate_position(uint32_t x_, uint32_t y_, uint32_t layer_)
      : x(x_)
      , y(y_)
      , layer(layer_)
    {
      Expects(layer < number_of_layers);
      Expects(x < size_of_layer(layer));
      Expects(y < size_of_layer(layer));
    }

    explicit constexpr coordinate_position(integer_position i)
      : coordinate_position(i.x(), i.y(), i.layer())
    {}

    [[nodiscard]] constexpr bool operator==(const coordinate_position& other) const
    {
      return x == other.x && y == other.y && layer == other.layer;
    }

  private:
    uint32_t x;
    uint32_t y;
    uint32_t layer;
  };

  using position_set = std::bitset<32>;

  constexpr position_set all_positions() { return position_set(0x3FFFFFFF); }
} // namespace pylos

using namespace pylos;

TEST_CASE("Board positions")
{
  STATIC_REQUIRE(number_of_balls_in_lower_layers_[number_of_layers] == 30);

  for(uint32_t l = 0; l < number_of_layers; l++)
  {
    REQUIRE(number_of_balls_in_layer(l) == size_of_layer(l) * size_of_layer(l));
  }

  REQUIRE(all_positions().count() == 30);
}
