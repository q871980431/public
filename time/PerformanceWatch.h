#ifndef _PerformanceWatch_H_
#define _PerformanceWatch_H_
#include "MultiSys.h"
#include <chrono>
#include <sstream>
#include <vector>

typedef std::vector<int32_t> TickPoints;
struct PerformanceInfo 
{
	s32 maxTick;
	s32 avrgTick;
	s64 total;

};

class PerformanceView 
{
public:
	PerformanceView(s32 size, s32 rate) {
		_size = size * rate / 1000;
		_total = 0;
		_max = 0;
	}
	
	void AddTick(s32 idx, s32 tick)
	{
		if (idx < _size)
		{
			_total += tick;
			if (tick > _max)
				_max = tick;
		}
	}

	PerformanceInfo CreatePerformanceInfo()
	{
		PerformanceInfo info;
		info.maxTick = _max;
		info.total = _total / 1000;
		info.avrgTick = _total / _size;
		return info;
	}
private:
	s32 _size;
	s64 _total;
	s32 _max;
};

template<typename T, int32_t TIME_OUT = 3>
class PerformanceWatch
{
public:
	PerformanceWatch() { Start(); }
	inline void Start() { _watch.Reset(); }
	inline void Record() {_tickPoints.push_back((s32)(_watch.Interval()));}
	inline void Clean() { _tickPoints.clear(); };
	std::string CreatePerformanceInfo()
	{
		int64_t maxTick = 0;
		int64_t minTick = 0;
		int64_t avrgTick = 0;
		int64_t timoutCount = 0;
		int64_t total = 0;

		if (_tickPoints.size() == 0)
			return "no date need create";

		std::sort(_tickPoints.begin(), _tickPoints.end());
		size_t size = _tickPoints.size();
		PerformanceView performanceView990(size, 990);
		PerformanceView performanceView995(size, 995);
		maxTick = _tickPoints[size - 1];
		minTick = _tickPoints[0];
		for (size_t i = 0; i < size; i++)
		{
			total += _tickPoints[i];
			if (_tickPoints[i] >= TIME_OUT)
				timoutCount++;
			performanceView990.AddTick(i, _tickPoints[i]);
			performanceView995.AddTick(i, _tickPoints[i]);
		}
		avrgTick = total / size;
		std::stringstream ostream;
		ostream << "total info, minTick:" << minTick  << " maxTick:" << maxTick << ", avrgTick:" << avrgTick << ", totoal(scaled-down1000):" << total / 1000;
		ostream << ", timeout:" << timoutCount << "\n";
		ostream << "c990 info, ";
		BuildPerformanceInfo(ostream, performanceView990.CreatePerformanceInfo());
		ostream << "c995 info, ";
		BuildPerformanceInfo(ostream, performanceView995.CreatePerformanceInfo());
		return ostream.str();
	}
private:
	void BuildPerformanceInfo(std::stringstream &ostream, const PerformanceInfo &performanceInfo) const
	{
		ostream << "maxTick:" << performanceInfo.maxTick << ", avrgTick:" << performanceInfo.avrgTick;
		ostream << ", total(scaled-down1000):" << performanceInfo.total << "\n";
	}
protected:
private:
	int64_t			_tick;
	TickPoints		_tickPoints;
	T				_watch;
};

#endif