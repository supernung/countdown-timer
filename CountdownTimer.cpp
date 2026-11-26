#include "CountdownTimer.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>

using namespace ffglex;

// ---------------------------------------------------------------- Info plugin
static CFFGLPluginInfo PluginInfo(
	PluginFactory< CountdownTimer >,
	"CDTM",              // ID plugin (harus unik, 4 karakter)
	"Countdown Timer",   // nama di Resolume
	2, 1,                // versi FFGL API
	1, 0,                // versi plugin
	FF_SOURCE,           // jenis: source
	"Countdown timer dengan notes dan teks kedip",
	"Dibuat dengan Claude" );

// ---------------------------------------------------------------- Parameter
enum Param
{
	P_HOURS, P_MINUTES, P_SECONDS,
	P_RUN, P_RESET,
	P_NOTES, P_FINISH_TEXT, P_BLINK,
	P_TXT_R, P_TXT_G, P_TXT_B,
	P_FIN_R, P_FIN_G, P_FIN_B,
	P_BG_R, P_BG_G, P_BG_B, P_BG_A,
	P_COUNT
};

static const int kClockCap  = 8;   // "HH:MM:SS"
static const int kFinishCap = 32;  // maksimal karakter teks habis
static const int kNotesCap  = 64;  // maksimal karakter notes
static const int kCharCount = kClockCap + kFinishCap + kNotesCap;

// ---------------------------------------------------------------- Font 5x7 (ASCII 32..126)
// Tiap glyph 5 kolom, bit 0 = baris paling atas.
static const uint8_t kFont[95][5] = {
	{0x00,0x00,0x00,0x00,0x00},{0x00,0x00,0x5F,0x00,0x00},{0x00,0x07,0x00,0x07,0x00},{0x14,0x7F,0x14,0x7F,0x14},
	{0x24,0x2A,0x7F,0x2A,0x12},{0x23,0x13,0x08,0x64,0x62},{0x36,0x49,0x55,0x22,0x50},{0x00,0x05,0x03,0x00,0x00},
	{0x00,0x1C,0x22,0x41,0x00},{0x00,0x41,0x22,0x1C,0x00},{0x14,0x08,0x3E,0x08,0x14},{0x08,0x08,0x3E,0x08,0x08},
	{0x00,0x50,0x30,0x00,0x00},{0x08,0x08,0x08,0x08,0x08},{0x00,0x60,0x60,0x00,0x00},{0x20,0x10,0x08,0x04,0x02},
	{0x3E,0x51,0x49,0x45,0x3E},{0x00,0x42,0x7F,0x40,0x00},{0x42,0x61,0x51,0x49,0x46},{0x21,0x41,0x45,0x4B,0x31},
	{0x18,0x14,0x12,0x7F,0x10},{0x27,0x45,0x45,0x45,0x39},{0x3C,0x4A,0x49,0x49,0x30},{0x01,0x71,0x09,0x05,0x03},
	{0x36,0x49,0x49,0x49,0x36},{0x06,0x49,0x49,0x29,0x1E},{0x00,0x36,0x36,0x00,0x00},{0x00,0x56,0x36,0x00,0x00},
	{0x08,0x14,0x22,0x41,0x00},{0x14,0x14,0x14,0x14,0x14},{0x00,0x41,0x22,0x14,0x08},{0x02,0x01,0x51,0x09,0x06},
	{0x32,0x49,0x79,0x41,0x3E},{0x7E,0x11,0x11,0x11,0x7E},{0x7F,0x49,0x49,0x49,0x36},{0x3E,0x41,0x41,0x41,0x22},
	{0x7F,0x41,0x41,0x22,0x1C},{0x7F,0x49,0x49,0x49,0x41},{0x7F,0x09,0x09,0x09,0x01},{0x3E,0x41,0x49,0x49,0x7A},
	{0x7F,0x08,0x08,0x08,0x7F},{0x00,0x41,0x7F,0x41,0x00},{0x20,0x40,0x41,0x3F,0x01},{0x7F,0x08,0x14,0x22,0x41},
	{0x7F,0x40,0x40,0x40,0x40},{0x7F,0x02,0x0C,0x02,0x7F},{0x7F,0x04,0x08,0x10,0x7F},{0x3E,0x41,0x41,0x41,0x3E},
	{0x7F,0x09,0x09,0x09,0x06},{0x3E,0x41,0x51,0x21,0x5E},{0x7F,0x09,0x19,0x29,0x46},{0x46,0x49,0x49,0x49,0x31},
	{0x01,0x01,0x7F,0x01,0x01},{0x3F,0x40,0x40,0x40,0x3F},{0x1F,0x20,0x40,0x20,0x1F},{0x3F,0x40,0x38,0x40,0x3F},
	{0x63,0x14,0x08,0x14,0x63},{0x07,0x08,0x70,0x08,0x07},{0x61,0x51,0x49,0x45,0x43},{0x00,0x7F,0x41,0x41,0x00},
	{0x02,0x04,0x08,0x10,0x20},{0x00,0x41,0x41,0x7F,0x00},{0x04,0x02,0x01,0x02,0x04},{0x40,0x40,0x40,0x40,0x40},
	{0x00,0x01,0x02,0x04,0x00},{0x20,0x54,0x54,0x54,0x78},{0x7F,0x48,0x44,0x44,0x38},{0x38,0x44,0x44,0x44,0x20},
	{0x38,0x44,0x44,0x48,0x7F},{0x38,0x54,0x54,0x54,0x18},{0x08,0x7E,0x09,0x01,0x02},{0x0C,0x52,0x52,0x52,0x3E},
	{0x7F,0x08,0x04,0x04,0x78},{0x00,0x44,0x7D,0x40,0x00},{0x20,0x40,0x44,0x3D,0x00},{0x7F,0x10,0x28,0x44,0x00},
	{0x00,0x41,0x7F,0x40,0x00},{0x7C,0x04,0x18,0x04,0x78},{0x7C,0x08,0x04,0x04,0x78},{0x38,0x44,0x44,0x44,0x38},
	{0x7C,0x14,0x14,0x14,0x08},{0x08,0x14,0x14,0x18,0x7C},{0x7C,0x08,0x04,0x04,0x08},{0x48,0x54,0x54,0x54,0x20},
	{0x04,0x3F,0x44,0x40,0x20},{0x3C,0x40,0x40,0x20,0x7C},{0x1C,0x20,0x40,0x20,0x1C},{0x3C,0x40,0x30,0x40,0x3C},
	{0x44,0x28,0x10,0x28,0x44},{0x0C,0x50,0x50,0x50,0x3C},{0x44,0x64,0x54,0x4C,0x44},{0x00,0x08,0x36,0x41,0x00},
	{0x00,0x00,0x7F,0x00,0x00},{0x00,0x41,0x36,0x08,0x00},{0x10,0x08,0x08,0x10,0x08}
};

// ---------------------------------------------------------------- Shader
static const char* kVertexShader = R"(#version 410 core
layout( location = 0 ) in vec4 vPosition;
layout( location = 1 ) in vec2 vUV;
out vec2 uv;
void main()
{
	gl_Position = vPosition;
	uv = vUV;
}
)";

// Tiap baris teks: uLine = (x0, y0, pixel per unit font, jumlah karakter), dalam piksel layar.
// Baris 0 = jam, 1 = teks habis, 2 = notes.
static const char* kFragmentShader = R"(#version 410 core
uniform sampler2D uFont;
uniform int  uChars[104];
uniform vec4 uLine[3];
uniform vec4 uLineCol[3];
uniform vec4 uBg;
out vec4 fragColor;

float glyphPix( int code, ivec2 g )
{
	int idx = code - 32;
	if( idx < 0 || idx > 94 ) return 0.0;
	return texelFetch( uFont, ivec2( ( idx % 16 ) * 8 + g.x, ( idx / 16 ) * 8 + g.y ), 0 ).r;
}

float drawLine( vec2 frag, vec4 L, int base )
{
	if( L.w < 1.0 ) return 0.0;
	vec2 p = ( frag - L.xy ) / L.z;
	if( p.x < 0.0 || p.y < 0.0 || p.y >= 8.0 || p.x >= 6.0 * L.w ) return 0.0;
	int ci = int( floor( p.x / 6.0 ) );
	int gx = int( floor( mod( p.x, 6.0 ) ) );
	int gy = 7 - int( floor( p.y ) );
	if( gx > 4 ) return 0.0;
	return glyphPix( uChars[ base + ci ], ivec2( gx, gy ) );
}

void main()
{
	vec4 col = vec4( uBg.rgb * uBg.a, uBg.a );
	if( drawLine( gl_FragCoord.xy, uLine[0], 0  ) > 0.5 ) col = vec4( uLineCol[0].rgb, 1.0 );
	if( drawLine( gl_FragCoord.xy, uLine[1], 8  ) > 0.5 ) col = vec4( uLineCol[1].rgb, 1.0 );
	if( drawLine( gl_FragCoord.xy, uLine[2], 40 ) > 0.5 ) col = vec4( uLineCol[0].rgb, 1.0 );
	fragColor = col;
}
)";

// ---------------------------------------------------------------- Helper
static int FillChars( int* dst, int cap, const std::string& s )
{
	int n = 0;
	for( unsigned char c : s )
	{
		if( c < 32 || c > 126 ) continue; // lewati karakter non-ASCII
		if( n >= cap ) break;
		dst[ n++ ] = c;
	}
	return n;
}

// Skala piksel per unit font supaya teks muat lebar layar dan tidak lebih tinggi dari maxH.
static float FitScale( float W, int len, float maxH )
{
	if( len < 1 ) return 1.0f;
	return std::max( 1.0f, std::min( 0.9f * W / ( 6.0f * len ), maxH / 8.0f ) );
}

// ---------------------------------------------------------------- Plugin
CountdownTimer::CountdownTimer()
{
	SetMinInputs( 0 );
	SetMaxInputs( 0 );

	std::memset( params_, 0, sizeof( params_ ) );
	params_[ P_MINUTES ] = 5.0f / 59.0f; // default 5 menit
	params_[ P_BLINK ]   = 0.3f;
	params_[ P_TXT_R ] = params_[ P_TXT_G ] = params_[ P_TXT_B ] = 1.0f;
	params_[ P_FIN_R ] = 1.0f;
	params_[ P_FIN_G ] = 0.15f;
	params_[ P_FIN_B ] = 0.15f;
	notes_      = "Catatan";
	finishText_ = "WAKTU HABIS";

	SetParamInfo( P_HOURS,   "Hours (0-99)",    FF_TYPE_STANDARD, params_[ P_HOURS ] );
	SetParamInfo( P_MINUTES, "Minutes (0-59)",  FF_TYPE_STANDARD, params_[ P_MINUTES ] );
	SetParamInfo( P_SECONDS, "Seconds (0-59)",  FF_TYPE_STANDARD, params_[ P_SECONDS ] );
	SetParamInfo( P_RUN,     "Run",             FF_TYPE_BOOLEAN,  false );
	SetParamInfof( P_RESET,  "Reset",           FF_TYPE_EVENT );
	SetParamInfo( P_NOTES,       "Notes",       FF_TYPE_TEXT, notes_.c_str() );
	SetParamInfo( P_FINISH_TEXT, "Finish Text", FF_TYPE_TEXT, finishText_.c_str() );
	SetParamInfo( P_BLINK,   "Blink Speed",     FF_TYPE_STANDARD, params_[ P_BLINK ] );

	SetParamInfo( P_TXT_R, "Text Color",   FF_TYPE_RED,   params_[ P_TXT_R ] );
	SetParamInfo( P_TXT_G, "Text Green",   FF_TYPE_GREEN, params_[ P_TXT_G ] );
	SetParamInfo( P_TXT_B, "Text Blue",    FF_TYPE_BLUE,  params_[ P_TXT_B ] );
	SetParamInfo( P_FIN_R, "Finish Color", FF_TYPE_RED,   params_[ P_FIN_R ] );
	SetParamInfo( P_FIN_G, "Finish Green", FF_TYPE_GREEN, params_[ P_FIN_G ] );
	SetParamInfo( P_FIN_B, "Finish Blue",  FF_TYPE_BLUE,  params_[ P_FIN_B ] );
	SetParamInfo( P_BG_R,  "Background",   FF_TYPE_RED,   params_[ P_BG_R ] );
	SetParamInfo( P_BG_G,  "BG Green",     FF_TYPE_GREEN, params_[ P_BG_G ] );
	SetParamInfo( P_BG_B,  "BG Blue",      FF_TYPE_BLUE,  params_[ P_BG_B ] );
	SetParamInfo( P_BG_A,  "BG Opacity",   FF_TYPE_STANDARD, params_[ P_BG_A ] );

	last_ = finishTime_ = Clock::now();
	ReloadTime();
}

CountdownTimer::~CountdownTimer() {}

int CountdownTimer::TotalSeconds() const
{
	int h = (int)std::lround( params_[ P_HOURS ] * 99.0f );
	int m = (int)std::lround( params_[ P_MINUTES ] * 59.0f );
	int s = (int)std::lround( params_[ P_SECONDS ] * 59.0f );
	return h * 3600 + m * 60 + s;
}

void CountdownTimer::ReloadTime()
{
	remaining_ = TotalSeconds();
	finished_  = false;
}

FFResult CountdownTimer::InitGL( const FFGLViewportStruct* )
{
	if( !shader_.Compile( kVertexShader, kFragmentShader ) )
	{
		DeInitGL();
		return FF_FAIL;
	}
	if( !quad_.Initialise() )
	{
		DeInitGL();
		return FF_FAIL;
	}

	// Atlas font 16x6 sel, tiap sel 8x8 piksel -> 128x48, 1 channel.
	std::vector< uint8_t > atlas( 128 * 48, 0 );
	for( int i = 0; i < 95; ++i )
		for( int col = 0; col < 5; ++col )
			for( int row = 0; row < 7; ++row )
				if( ( kFont[ i ][ col ] >> row ) & 1 )
					atlas[ ( ( i / 16 ) * 8 + row ) * 128 + ( i % 16 ) * 8 + col ] = 255;

	glGenTextures( 1, &fontTex_ );
	glBindTexture( GL_TEXTURE_2D, fontTex_ );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_R8, 128, 48, 0, GL_RED, GL_UNSIGNED_BYTE, atlas.data() );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glBindTexture( GL_TEXTURE_2D, 0 );
	return FF_SUCCESS;
}

FFResult CountdownTimer::DeInitGL()
{
	shader_.FreeGLResources();
	quad_.Release();
	if( fontTex_ )
	{
		glDeleteTextures( 1, &fontTex_ );
		fontTex_ = 0;
	}
	return FF_SUCCESS;
}

FFResult CountdownTimer::ProcessOpenGL( ProcessOpenGLStruct* )
{
	// --- update timer
	const auto now = Clock::now();
	if( running_ && !finished_ )
	{
		remaining_ -= std::chrono::duration< double >( now - last_ ).count();
		if( remaining_ <= 0.0 )
		{
			remaining_  = 0.0;
			finished_   = true;
			finishTime_ = now;
		}
	}
	last_ = now;

	// --- siapkan teks
	int chars[ kCharCount ];
	std::fill( chars, chars + kCharCount, 32 );

	char clock[ 16 ];
	int tot = (int)std::ceil( remaining_ - 1e-9 );
	std::snprintf( clock, sizeof( clock ), "%02d:%02d:%02d", std::min( tot / 3600, 99 ), ( tot / 60 ) % 60, tot % 60 );
	int clockLen  = FillChars( chars, kClockCap, clock );
	int finishLen = 0;
	if( finished_ )
	{
		double hz = 0.5 + params_[ P_BLINK ] * 9.5; // 0.5 - 10 kedip per detik
		double t  = std::chrono::duration< double >( now - finishTime_ ).count();
		if( std::fmod( t * hz, 1.0 ) < 0.5 )
			finishLen = FillChars( chars + kClockCap, kFinishCap, finishText_ );
	}
	int notesLen = FillChars( chars + kClockCap + kFinishCap, kNotesCap, notes_ );

	// --- layout (dalam piksel)
	GLint vp[ 4 ];
	glGetIntegerv( GL_VIEWPORT, vp );
	const float W = (float)vp[ 2 ], H = (float)vp[ 3 ];

	float scClock  = FitScale( W, clockLen, 0.40f * H );
	float scFinish = FitScale( W, finishLen, 0.16f * H );
	float scNotes  = FitScale( W, notesLen, 0.09f * H );

	float lines[ 12 ] = {
		( W - 6.0f * clockLen * scClock ) * 0.5f,   0.52f * H, scClock,  (float)clockLen,
		( W - 6.0f * finishLen * scFinish ) * 0.5f, 0.30f * H, scFinish, (float)finishLen,
		( W - 6.0f * notesLen * scNotes ) * 0.5f,   0.08f * H, scNotes,  (float)notesLen };
	float cols[ 12 ] = {
		params_[ P_TXT_R ], params_[ P_TXT_G ], params_[ P_TXT_B ], 1.0f,
		params_[ P_FIN_R ], params_[ P_FIN_G ], params_[ P_FIN_B ], 1.0f,
		params_[ P_TXT_R ], params_[ P_TXT_G ], params_[ P_TXT_B ], 1.0f };

	// --- gambar
	ScopedShaderBinding shaderBinding( shader_.GetGLID() );
	const GLuint prog = shader_.GetGLID();

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, fontTex_ );
	glUniform1i( glGetUniformLocation( prog, "uFont" ), 0 );
	glUniform1iv( glGetUniformLocation( prog, "uChars" ), kCharCount, chars );
	glUniform4fv( glGetUniformLocation( prog, "uLine" ), 3, lines );
	glUniform4fv( glGetUniformLocation( prog, "uLineCol" ), 3, cols );
	glUniform4f( glGetUniformLocation( prog, "uBg" ), params_[ P_BG_R ], params_[ P_BG_G ], params_[ P_BG_B ], params_[ P_BG_A ] );

	glDisable( GL_BLEND );
	quad_.Draw();
	glBindTexture( GL_TEXTURE_2D, 0 );
	return FF_SUCCESS;
}

FFResult CountdownTimer::SetFloatParameter( unsigned int index, float value )
{
	if( index >= P_COUNT ) return FF_FAIL;
	const float prev = params_[ index ];
	params_[ index ] = value;

	switch( index )
	{
	case P_HOURS:
	case P_MINUTES:
	case P_SECONDS:
		ReloadTime(); // edit waktu langsung memuat ulang sisa waktu
		break;
	case P_RUN:
		if( value > 0.5f && prev <= 0.5f )
		{
			if( finished_ || remaining_ <= 0.0 ) ReloadTime();
			running_ = true;
			last_    = Clock::now();
		}
		else if( value <= 0.5f )
		{
			running_ = false; // pause
		}
		break;
	case P_RESET:
		if( value > 0.5f && prev <= 0.5f ) ReloadTime();
		break;
	}
	return FF_SUCCESS;
}

float CountdownTimer::GetFloatParameter( unsigned int index )
{
	return index < P_COUNT ? params_[ index ] : 0.0f;
}

FFResult CountdownTimer::SetTextParameter( unsigned int index, const char* value )
{
	if( !value ) return FF_FAIL;
	if( index == P_NOTES )       notes_      = value;
	else if( index == P_FINISH_TEXT ) finishText_ = value;
	else return FF_FAIL;
	return FF_SUCCESS;
}

char* CountdownTimer::GetTextParameter( unsigned int index )
{
	if( index == P_NOTES )       return const_cast< char* >( notes_.c_str() );
	if( index == P_FINISH_TEXT ) return const_cast< char* >( finishText_.c_str() );
	return nullptr;
}
