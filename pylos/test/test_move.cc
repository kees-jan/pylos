#include <array>
#include <bitset>
#include <cassert>
#include <gsl/gsl_assert>
#include <gsl/gsl_util>
#include <iostream>
#include <set>

#include <fmt/core.h>

#include <catch2/catch.hpp>
#include <cppcoro/generator.hpp>

#define Bug() GSL_CONTRACT_CHECK("Bug", false)

namespace pylos
{
  constexpr uint32_t                               number_of_layers    = 4;
  constexpr uint32_t                               number_of_positions = 30;
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

  [[nodiscard]] constexpr auto number_of_balls_in_lower_layers(uint32_t l)
  {
    return gsl::at(number_of_balls_in_lower_layers_, l);
  }

  [[nodiscard]] constexpr auto size_of_layer(uint32_t l)
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
      Expects(i < number_of_positions);
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
      Expects(position < number_of_positions);
      Expects(number_of_balls_in_lower_layers(number_of_layers) == number_of_positions);

      for(uint32_t l = 0; l < number_of_layers; l++)
      {
        if(number_of_balls_in_lower_layers(l + 1) > position)
        {
          return l;
        }
      }

      // Can't happen
      return std::numeric_limits<uint32_t>::max();
    }

    bool operator==(const integer_position& other) const { return position == other.position; }

    const uint32_t position;
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

    explicit constexpr operator integer_position()
    {
      return integer_position(number_of_balls_in_lower_layers(layer) + x + y * size_of_layer(layer));
    }

    [[nodiscard]] constexpr bool operator==(const coordinate_position& other) const
    {
      return x == other.x && y == other.y && layer == other.layer;
    }

    const uint32_t x;
    const uint32_t y;
    const uint32_t layer;
  };

  constexpr auto integer_position_comparator = [](const integer_position& left, const integer_position& right) {
    return left.position < right.position;
  };

  constexpr auto coordinate_position_comparator = [](const coordinate_position& left, const coordinate_position& right) {
    return std::make_tuple(left.layer, left.x, left.y) < std::make_tuple(right.layer, right.x, right.y);
  };

  using position_set            = std::bitset<32>;
  using integer_position_set    = std::set<integer_position, decltype(integer_position_comparator)>;
  using coordinate_position_set = std::set<coordinate_position, decltype(coordinate_position_comparator)>;

  constexpr position_set all_positions() { return position_set(0x3FFFFFFFu); }

  cppcoro::generator<integer_position> integer_positions(const position_set positions)
  {
    for(uint32_t i = 0; i < positions.size(); i++)
    {
      if(positions[i])
      {
        co_yield integer_position(i);
      }
    }
  }

} // namespace pylos


namespace pylos
{
  std::ostream& operator<<(std::ostream& os, const integer_position& p) { return os << p.position; }

  std::ostream& operator<<(std::ostream& os, const coordinate_position& p)
  {
    return os << fmt::format("{{}, {}, {}}", p.layer, p.x, p.y);
  }
} // namespace pylos

using namespace pylos;

TEST_CASE("Board position numbers")
{
  STATIC_REQUIRE(number_of_balls_in_lower_layers_[number_of_layers] == number_of_positions);

  uint32_t s = 0;
  for(uint32_t l = 0; l < number_of_layers; l++)
  {
    REQUIRE(number_of_balls_in_layer(l) == size_of_layer(l) * size_of_layer(l));
    REQUIRE(number_of_balls_in_lower_layers(l) == s);
    s += number_of_balls_in_layer(l);
  }
}

TEST_CASE("Board position counts")
{
  REQUIRE(all_positions().count() == number_of_positions);

  integer_position_set    ips;
  coordinate_position_set cps;

  for(const auto& ip: integer_positions(all_positions()))
  {
    auto cp  = static_cast<coordinate_position>(ip);
    auto ip2 = static_cast<integer_position>(cp);
    CHECK(ip == ip2);
    CHECK(cp == cp);
    ips.insert(ip);
    cps.insert(cp);
  }

  CHECK(ips.size() == number_of_positions);
  CHECK(cps.size() == number_of_positions);
}
