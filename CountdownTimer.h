#pragma once
#include <chrono>
#include <string>
#include "FFGLSDK.h"

// Countdown Timer: FFGL source untuk Resolume.
// Jam/menit/detik bisa diedit, ada notes di bawah, teks kedip saat waktu habis, dan pengaturan warna.
class CountdownTimer : public CFFGLPlugin
{
public:
	CountdownTimer();
	~CountdownTimer() override;

	FFResult InitGL( const FFGLViewportStruct* vp ) override;
	FFResult ProcessOpenGL( ProcessOpenGLStruct* pGL ) override;
	FFResult DeInitGL() override;

	FFResult SetFloatParameter( unsigned int index, float value ) override;
	float GetFloatParameter( unsigned int index ) override;
	FFResult SetTextParameter( unsigned int index, const char* value ) override;
	char* GetTextParameter( unsigned int index ) override;

private:
	using Clock = std::chrono::steady_clock;

	int TotalSeconds() const;
	void ReloadTime();

	// Parameter (nilai 0..1 dari host, kecuali teks)
	float params_[32];
	std::string notes_;
	std::string finishText_;

	// State timer
	bool running_      = false;
	bool finished_     = false;
	double remaining_  = 0.0;
	Clock::time_point last_;
	Clock::time_point finishTime_;

	// Resource OpenGL
	ffglex::FFGLShader shader_;
	ffglex::FFGLScreenQuad quad_;
	GLuint fontTex_ = 0;
};
