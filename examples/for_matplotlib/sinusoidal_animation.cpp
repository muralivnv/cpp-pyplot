#include "../../include/cppyplot.hpp"
#include <algorithm>

int main()
{
  Cppyplot::cppyplot pyp;

  std::vector<float> vec(500, 0.0F);
  std::vector<float> sine;
  sine.reserve(500);
  std::iota(vec.begin(), vec.end(), 0.0F);
  std::transform(vec.begin(), vec.end(), std::back_inserter(sine), 
                [](auto& elem){return std::sinf(elem * 3.14159265F/180.0F);});

  pyp.raw(R"pyp(
    
  fig = plt.figure(figsize=(6,5))
  ax = plt.axes(xlim=(0, 500), ylim=(-1.5,1.5))
  line, = ax.plot([], [], lw=1.5)
  plt.grid(True)
  plt.xlabel("Angle (deg)", fontsize=12)
  plt.ylabel("Sine", fontsize=12)
  plt.title("Sinusoidal",fontsize=14)
  
  def anim_init():
    line.set_data([], [])
    return line,

  def anim_update(frame):
    x = vec[0:frame]
    y = sine[0:frame]
    line.set_data(x,y)
    return line,

  anim = FuncAnimation(fig, anim_update, init_func=anim_init, 
                       frames=500, interval=2, blit=True)

  plt.show()
  )pyp", _p(vec), _p(sine));

}