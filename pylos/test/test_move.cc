#include <array>
#include <bitset>
#include <cassert>

#include <catch2/catch.hpp>

namespace pylos
{
  constexpr uint32_t                               number_of_layers = 4;
  constexpr std::array<uint32_t, number_of_layers> number_of_balls_in_layer{16, 9, 4, 1};

  constexpr auto number_of_balls_in_lower_layers = [] {
    std::array<uint32_t, number_of_layers + 1> t{};
    uint32_t                                   sum{0};

    for(uint32_t i = 0; i < number_of_layers; i++)
    {
      t[i] = sum;
      sum += number_of_balls_in_layer[i];
    }
    t[number_of_layers] = sum;
    return t;
  }();

  constexpr uint32_t size_of_layer(uint32_t l)
  {
    assert(l < number_of_layers);
    return number_of_layers - l;
  }

  class integer_position
  {
  public:
    explicit constexpr integer_position(uint32_t i)
      : position(i)
    {
      assert(i < 30);
    }

    constexpr uint32_t x() { return (position - number_of_balls_in_lower_layers[layer()]) % size_of_layer(layer()); }

    constexpr uint32_t y() { return (position - number_of_balls_in_lower_layers[layer()]) / size_of_layer(layer()); }

    constexpr uint32_t layer()
    {
      for(uint32_t l = 0; l < number_of_layers; l++)
      {
        if(number_of_balls_in_lower_layers[l + 1] > position)
        {
          return l;
        }
      }

      assert(false);
    }


  private:
    uint32_t position;
  };

  class coordinate_position
  {
  public:
    constexpr coordinate_position(uint32_t x_, uint32_t y_, uint32_t layer_)
      : x(x_)
      , y(y_)
      , layer(layer_)
    {
      assert(layer < number_of_layers);
      assert(x < size_of_layer(layer));
      assert(y < size_of_layer(layer));
    }

    explicit constexpr coordinate_position(integer_position i)
      : coordinate_position(i.x(), i.y(), i.layer())
    {}

    constexpr bool operator==(const coordinate_position& other) { return x == other.x && y == other.y && layer == other.layer; }

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
  STATIC_REQUIRE(number_of_balls_in_lower_layers[number_of_layers] == 30);

  for(uint32_t l = 0; l < number_of_layers; l++)
  {
    REQUIRE(number_of_balls_in_layer[l] == size_of_layer(l) * size_of_layer(l));
  }

  REQUIRE(all_positions().count() == 30);
}
