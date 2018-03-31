#pragma once

template <typename T, size_t SIZE>
struct MovingAverage
{
	MovingAverage() { reset(); }
	void reset() { idx = 0; sum = 0; for(T& it : buf) it=0; }
	T get() const { return sum / SIZE; }
	void add(T v)
	{
		sum += v;
		sum -= buf[idx];
		buf[idx] = v;
		idx = (idx + 1) % SIZE;
	}
	size_t idx;
	T sum;
	T buf[SIZE];
};

template <typename T, size_t S>
struct TimingScope
{

	TimingScope(MovingAverage<T, S>& output)
		: m_output(output)
	{}

	~TimingScope()
	{
		m_output.add(m_timer.time());
	}

	MovingAverage<T, S>& m_output;
	Timer m_timer;
};

