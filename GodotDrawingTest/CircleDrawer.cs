using Godot;
using System;

public partial class CircleDrawer : Godot.Node2D
{
	private int _numFrames = 0;
	private float _time = 0;

    int xmin = 0;
	int xmax = 1920;
    int ymin = 0;
    int ymax = 1080;
    int rmin = 1;
    int rmax = 5;
	int numCircles = 50000;

	// Called every frame. 'delta' is the elapsed time since the previous frame.
	public override void _Process(double delta)
	{
		_numFrames++;
		_time += (float)delta;
		if (_time > 1)
		{
			GD.Print("FPS: " + _numFrames);
			_numFrames = 0;
			_time = 0;
		}

	}

    public override void _Draw()
    {
		// draw random circles
		Random r = new Random();
		for (int i = 0; i < numCircles; i++)
		{
			DrawCircle(new Vector2(xmin + r.Next(0, xmax), ymin + r.Next(0, ymax)), r.Next(rmin, rmax), new Color(r.NextSingle(), r.NextSingle(), r.NextSingle(),r.NextSingle()));
		}
    }
}
