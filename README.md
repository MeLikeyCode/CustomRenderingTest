# Rendering Test
Project to test custom rendering, and how it can be **faster** than rendering provided by popular/industry-standard frameworks (for certain cases).

# Setup
I tested rendering lots of circles in godot, SFML, and my own rendering implementation (which uses OpenGL).

# Results
My own renderer can render 300,000 circles at 30 fps. SFML can only render 20,000, and godot 50,000. My renderer beats godot by 6 times the amount of circles it can render at 30 fps. 

The reason why my renderer is better is because it utilizes batching/instancing. Batching minimizes draw() calls (just one draw() call that draws all my circles instead of one draw() call per circle), and instancing minimizes GPU memory use (by consolidating shared attributes of the circles, i.e. the circle geometry/vertices).

# Lesson
The lesson here is, that **custom solutions** ("reinventing the wheel (or at least part of the wheel)") can sometimes be beneficial not just for learning, but actual performance. Often libraries are generalized to provide decent performence for a wide variety of cases, which hurts their performance in specialized cases.

