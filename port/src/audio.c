#include <PR/ultratypes.h>
#include <stdio.h>
#include <SDL.h>
#include "platform.h"
#include "config.h"
#include "audio.h"
#include "system.h"

#ifdef PLATFORM_WEB
#include <emscripten.h>
#endif

#ifndef PLATFORM_WEB
static SDL_AudioDeviceID dev;
#endif
static const s16 *nextBuf;
static u32 nextSize = 0;

#ifdef PLATFORM_WEB

// The browser has no audio device to queue to, so a block goes straight onto a
// Web Audio context: one AudioBufferSourceNode each, scheduled back to back.
// A context also has to be resumed from a user gesture before it will play,
// which is why the handlers below are installed.
//
// The schedule is kept further ahead than the native device buffer, because
// nothing can feed the browser's audio thread while a long frame is running on
// the main thread. audioGetBytesBuffered() takes that extra lead back off what
// it reports, so amgrFrame() goes on producing at its usual cadence.
#define WEB_AUDIO_RATE 22020
#define WEB_AUDIO_REPORT_SAMPLES 1100

static s32 bufferSize = 2048;

EM_JS(s32, webAudioInit, (s32 rate, s32 leadFrames, s32 reportFrames), {
	var AudioContextClass = window.AudioContext || window.webkitAudioContext;

	if (!AudioContextClass) {
		return -1;
	}

	var context = Module['pdAudioContext'];

	if (!context || context.state === 'closed') {
		// Queued blocks stay at the mixer's own rate; the browser resamples the
		// whole context output once, rather than every block separately.
		context = new AudioContextClass({ sampleRate: rate });
		Module['pdAudioContext'] = context;
	}

	if (context.sampleRate !== rate) {
		return -1;
	}

	var audio = {
		context: context,
		rate: rate,
		leadFrames: leadFrames,
		reportFrames: reportFrames,
		nextTime: 0,
		started: false,
		resuming: false
	};

	Module['pdAudio'] = audio;

	var resume = function () {
		if (context.state !== 'suspended' || audio.resuming) {
			return;
		}
		audio.resuming = true;
		context.resume().finally(function () {
			audio.resuming = false;
		});
	};

	document.addEventListener('pointerdown', resume, true);
	document.addEventListener('keydown', resume, true);
	resume();
	return 0;
});

EM_JS(s32, webAudioQueue, (const void *data, u32 size), {
	var audio = Module['pdAudio'];

	if (!audio || audio.context.state === 'closed') {
		return -1;
	}

	var frames = size >>> 2;
	var buffer = audio.context.createBuffer(2, frames, audio.rate);
	var left = buffer.getChannelData(0);
	var right = buffer.getChannelData(1);
	var src = data >>> 1;

	for (var i = 0; i < frames; i++) {
		left[i] = HEAP16[src++] / 32768;
		right[i] = HEAP16[src++] / 32768;
	}

	var now = audio.context.currentTime;
	var start = audio.nextTime;

	// Behind the clock means the schedule ran dry; start a new lead rather
	// than asking for a time that has already passed.
	if (!audio.started || start < now) {
		start = now + audio.leadFrames / audio.rate;
		audio.started = true;
	}

	var source = audio.context.createBufferSource();
	source.buffer = buffer;
	source.connect(audio.context.destination);
	source.onended = function () {
		source.disconnect();
	};
	source.start(start);
	audio.nextTime = start + buffer.duration;
	return 0;
});

EM_JS(u32, webAudioGetBytesBuffered, (), {
	var audio = Module['pdAudio'];

	if (!audio) {
		return 0;
	}

	var frames = Math.max(0,
		Math.floor((audio.nextTime - audio.context.currentTime) * audio.rate));
	var extra = Math.max(0, audio.leadFrames - audio.reportFrames);
	return Math.max(0, frames - extra) * 4;
});

#else
static s32 bufferSize = 512;
#endif
static s32 queueLimit = 8192;

s32 audioInit(void)
{
	nextBuf = NULL;

#ifdef PLATFORM_WEB
	if (webAudioInit(WEB_AUDIO_RATE, bufferSize, WEB_AUDIO_REPORT_SAMPLES) != 0) {
		sysLogPrintf(LOG_ERROR, "Web Audio init error");
		return -1;
	}

	return 0;
#else
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
		sysLogPrintf(LOG_ERROR, "SDL audio init error: %s", SDL_GetError());
		return -1;
	}

	SDL_AudioSpec want, have;
	SDL_zero(want);
	want.freq = 22020; // TODO: this might cause trouble for some platforms
	want.format = AUDIO_S16SYS;
	want.channels = 2;
	want.samples = bufferSize;
	want.callback = NULL;

	dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
	if (dev == 0) {
		sysLogPrintf(LOG_ERROR, "SDL_OpenAudio error: %s", SDL_GetError());
		return -1;
	}

	SDL_PauseAudioDevice(dev, 0);

	return 0;
#endif
}

s32 audioGetBytesBuffered(void)
{
#ifdef PLATFORM_WEB
	return webAudioGetBytesBuffered();
#else
	return SDL_GetQueuedAudioSize(dev);
#endif
}

s32 audioGetSamplesBuffered(void)
{
	return audioGetBytesBuffered() / 4;
}

void audioSetNextBuffer(const s16 *buf, u32 len)
{
	nextBuf = buf;
	nextSize = len;
}

void audioEndFrame(void)
{
	if (nextBuf && nextSize) {
		if (audioGetSamplesBuffered() < queueLimit) {
#ifdef PLATFORM_WEB
			webAudioQueue(nextBuf, nextSize);
#else
			SDL_QueueAudio(dev, nextBuf, nextSize);
#endif
		}
		nextBuf = NULL;
		nextSize = 0;
	}
}

PD_CONSTRUCTOR static void audioConfigInit(void)
{
#ifdef PLATFORM_WEB
	// Below a couple of thousand frames of lead the browser's audio thread
	// underruns whenever the main thread has a long frame.
	configRegisterInt("Audio.BufferSize", &bufferSize, 2048, 8192);
#else
	configRegisterInt("Audio.BufferSize", &bufferSize, 0, 1 * 1024 * 1024);
#endif
	configRegisterInt("Audio.QueueLimit", &queueLimit, 0, 1 * 1024 * 1024);
}
