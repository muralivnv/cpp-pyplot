#include "../../include/cppyplot.hpp"
#include <algorithm>
#include <random>

int main()
{
  Cppyplot::cppyplot pyp;

  std::vector<float> vec(500, 0.0F);
  std::vector<float> sine, cosine;
  sine.reserve(500);
  cosine.reserve(500);
  std::iota(vec.begin(), vec.end(), 0.0F);

  std::random_device seed;
  std::mt19937 gen(seed());
  std::uniform_real_distribution<float> uniform(0.0F, 1.0F);

  std::transform(vec.begin(), vec.end(), std::back_inserter(sine), 
                [&](auto& elem){return uniform(gen)*std::sinf(elem * 3.14159265F/180.0F);});

  std::transform(vec.begin(), vec.end(), std::back_inserter(cosine), 
                [&](auto& elem){return uniform(gen)*std::cosf(elem * 3.14159265F/180.0F);});

  pyp.raw(R"pyp(
    
  fig = plt.figure(figsize=(6,5))
  ax = plt.axes(xlim=(0, 500), ylim=(-1.5,1.5))
  line1, = ax.plot([], [], 'r-o', markersize=1, linewidth=0.8)
  line2, = ax.plot([], [], 'k-o', markersize=1, linewidth=0.8)
  plt.grid(True)
  plt.xlabel("Angle (deg)", fontsize=12)
  plt.title("Sine and Cosine",fontsize=14)
  
  def anim_init():
    line1.set_data([], [])
    line2.set_data([], [])
    return (line1, line2,)

  def anim_update(frame):
    x1 = vec[0:frame]
    y1 = sine[0:frame]
    x2 = vec[-frame:-1]
    y2 = cosine[-frame:-1] 
    line1.set_data(x1,y1)
    line2.set_data(x2,y2)
    return (line1,line2,)

  anim = FuncAnimation(fig, anim_update, init_func=anim_init, 
                       frames=500, interval=2, blit=True)
  anim.save("misc/sin.gif")
  plt.show()
  )pyp", _p(vec), _p(sine), _p(cosine));

}