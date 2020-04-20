#pragma once
class CircleGauge {
private:
	double Percentage;
public:
	CircleGauge();
	void Update(const double& NewVal);
	void View(const int X, const int Y);
};
