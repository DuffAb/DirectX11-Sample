//***************************************************************************************
// GameTimer.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include <windows.h>
#include "GameTimer.h"

GameTimer::GameTimer()
: mSecondsPerCount(0.0), mDeltaTime(-1.0), mBaseTime(0), 
  mPausedTime(0), mPrevTime(0), mCurrTime(0), mStopped(false)
{
	__int64 countsPerSec;
	// ��ȡ���ܼ�ʱ����Ƶ�ʣ�ÿ��ļ���������
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	mSecondsPerCount = 1.0 / (double)countsPerSec;
}

// Returns the total time elapsed since Reset() was called, NOT counting any
// time when the clock is stopped.
float GameTimer::TotalTime()const
{
	// If we are stopped, do not count the time that has passed since we stopped.
	// Moreover, if we previously already had a pause, the distance 
	// mStopTime - mBaseTime includes paused time, which we do not want to count.
	// To correct this, we can subtract the paused time from mStopTime:  
	// ���������ͣ״̬���������������ͣ��ʼ֮���ʱ�䡣
	// ����,�������֮ǰ�Ѿ��й���ͣ,��mStopTime - mBaseTime�������ͣʱ��, ���ǲ�����������ͣʱ�䣬
	// ��˻�Ҫ��ȥ��ͣʱ�䣺
	//
	//                     |<--paused time-->|
	// ----*---------------*-----------------*------------*------------*------> time
	//  mBaseTime       mStopTime        startTime     mStopTime    mCurrTime

	if( mStopped )
	{
		return (float)(((mStopTime - mPausedTime)-mBaseTime)*mSecondsPerCount);
	}

	// The distance mCurrTime - mBaseTime includes paused time,
	// which we do not want to count.  To correct this, we can subtract 
	// the paused time from mCurrTime:  
	// mCurrTime - mBaseTime������ͣʱ��,�����ǲ��������ͣʱ�䣬
	// ������Ǵ�mCurrTime��Ҫ��ȥmPausedTime��
	//
	//  (mCurrTime - mPausedTime) - mBaseTime 
	//
	//                     |<--paused time-->|
	// ----*---------------*-----------------*------------*------> time
	//  mBaseTime       mStopTime        startTime     mCurrTime
	
	else
	{
		return (float)(((mCurrTime-mPausedTime)-mBaseTime)*mSecondsPerCount);
	}
}

float GameTimer::DeltaTime()const
{
	return (float)mDeltaTime;
}

void GameTimer::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	mBaseTime = currTime;
	mPrevTime = currTime;
	mStopTime = 0;
	mStopped  = false;
}

void GameTimer::Start()
{
	__int64 startTime;
	// ��ȡ�Լ��������ĵ�ǰʱ��ֵ
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);


	// Accumulate the time elapsed between stop and start pairs.
	// �ۼ���ͣ�뿪ʼ֮�����ŵ�ʱ��
	//                     |<-------d------->|
	// ----*---------------*-----------------*------------> time
	//  mBaseTime       mStopTime        startTime     
	// ����Դ�����ͣ״̬
	if( mStopped )
	{
		// ���ۼ���ͣʱ��
		mPausedTime += (startTime - mStopTime);	

		// ��Ϊ�������¿�ʼ��ʱ�����mPrevTime��ֵ�Ͳ���ȷ�ˣ� Ҫ��������Ϊ��ǰʱ��
		mPrevTime = startTime;
		// ȡ����ͣ״̬
		mStopTime = 0;
		mStopped  = false;
	}
}

void GameTimer::Stop()
{
	// �����������ͣ״̬�����Թ�����Ĳ���
	if( !mStopped )
	{
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		// ��¼��ͣ��ʱ�䣬�����ñ�ʾ��ͣ״̬�ı�־
		mStopTime = currTime;
		mStopped  = true;
	}
}

void GameTimer::Tick()
{
	if( mStopped )
	{
		mDeltaTime = 0.0;
		return;
	}

	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	mCurrTime = currTime;

	// Time difference between this frame and the previous.
	// ��ǰ֡����һ֮֡���ʱ���
	mDeltaTime = (mCurrTime - mPrevTime)*mSecondsPerCount;

	// Prepare for next frame.
	// Ϊ������һ֡��׼��
	mPrevTime = mCurrTime;

	// Force nonnegative.  The DXSDK's CDXUTTimer mentions that if the 
	// processor goes into a power save mode or we get shuffled to another
	// processor, then mDeltaTime can be negative.
	// ȷ����Ϊ��ֵ��DXSDK�е�CDXUTTimer�ᵽ����������������˽ڵ�ģʽ
	// ���л�����һ����������mDeltaTime���Ϊ��ֵ
	if(mDeltaTime < 0.0)
	{
		mDeltaTime = 0.0;
	}
}

