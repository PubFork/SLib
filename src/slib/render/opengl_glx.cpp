/*
 *   Copyright (c) 2008-2018 SLIBIO <https://github.com/SLIBIO>
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *   THE SOFTWARE.
 */

#include "slib/render/opengl_glx.h"

#if defined(SLIB_RENDER_SUPPORT_OPENGL_GLX)

#include "slib/render/engine.h"
#include "slib/render/opengl.h"
#include "slib/core/thread.h"

#include "gl/GLX/glx.h"

namespace slib
{

	class _priv_GLXRendererImpl : public Renderer
	{
	public:
		Display* m_xdisplay;
		Window m_xwindow;
		GLXContext m_context;
		
		sl_bool m_flagRequestRender;
		AtomicRef<Thread> m_threadRender;

	public:
		_priv_GLXRendererImpl()
		{
			m_context = sl_null;
			m_flagRequestRender = sl_true;
		}

		~_priv_GLXRendererImpl()
		{
			release();
		}

	public:
		static Ref<_priv_GLXRendererImpl> create(Display* xdisplay, Window xwindow, const RendererParam& param)
		{
			if (!xdisplay) {
				return sl_null;
			}
			if (xwindow == None) {
				return sl_null;
			}
			
			GLint attrs[] = {
				GLX_RGBA,
				GLX_RED_SIZE, param.nRedBits,
				GLX_GREEN_SIZE, param.nGreenBits,
				GLX_BLUE_SIZE, param.nBlueBits,
				GLX_ALPHA_SIZE, param.nAlphaBits,
				GLX_DEPTH_SIZE, param.nDepthBits,
				GLX_STENCIL_SIZE, param.nStencilBits,
				GLX_DOUBLEBUFFER,
				None
			};
			
			XVisualInfo* xvinfo = glXChooseVisual(xdisplay, 0, attrs);
			if (!xvinfo) {
				return sl_null;
			}
			
			GLXContext context = glXCreateContext(xdisplay, xvinfo, NULL, GL_TRUE);
			if (!context) {
				return sl_null;
			}
			
			Ref<_priv_GLXRendererImpl> ret = new _priv_GLXRendererImpl();
			
			if (ret.isNotNull()) {
				ret->m_xdisplay = xdisplay;
				ret->m_xwindow = xwindow;
				ret->m_context = context;
				
				ret->m_threadRender = Thread::start(SLIB_FUNCTION_CLASS(_priv_GLXRendererImpl, run, ret.get()));
				
				ret->m_callback = param.callback;
				
				return ret;
			}
			
			glXDestroyContext(xdisplay, context);

			return sl_null;
		}

		void release()
		{
			ObjectLocker lock(this);
			
			Ref<Thread> thread = m_threadRender;
			if (thread.isNotNull()) {
				thread->finishAndWait();
				m_threadRender.setNull();
			}

			if (m_context) {
				glXDestroyContext(m_xdisplay, m_context);
				m_context = sl_null;
			}
		}

		void run()
		{
			glXMakeCurrent(m_xdisplay, m_xwindow, m_context);

			Ref<RenderEngine> engine = GL::createEngine();
			if (engine.isNull()) {
				return;
			}

			TimeCounter timer;
			while (Thread::isNotStoppingCurrent()) {
				runStep(engine.get());
				if (Thread::isNotStoppingCurrent()) {
					sl_uint64 t = timer.getElapsedMilliseconds();
					if (t < 10) {
						Thread::sleep(10 - (sl_uint32)(t));
					}
					timer.reset();
				} else {
					break;
				}
			}
			
			glXMakeCurrent(m_xdisplay, None, NULL);
		}

		void runStep(RenderEngine* engine)
		{
			PtrLocker<IRenderCallback> callback(m_callback);
			if (callback.isNotNull()) {
				XWindowAttributes attrs;
				if (!(XGetWindowAttributes(m_xdisplay, m_xwindow, &attrs))) {
					return;
				}
				if (attrs.map_state != IsViewable) {
					return;
				}
				sl_bool flagUpdate = sl_false;
				if (isRenderingContinuously()) {
					flagUpdate = sl_true;
				} else {
					if (m_flagRequestRender) {
						flagUpdate = sl_true;
					}
				}
				m_flagRequestRender = sl_false;
				if (flagUpdate) {
					if (attrs.width != 0 && attrs.height != 0) {
						engine->setViewport(0, 0, attrs.width, attrs.height);
						callback->onFrame(engine);
						glXSwapBuffers(m_xdisplay, m_xwindow);
					}
				}
			}
		}

		void requestRender()
		{
			m_flagRequestRender = sl_true;
		}

	};

	Ref<Renderer> GLX::createRenderer(void* xDisplay, unsigned long xWindow, const RendererParam& param)
	{
		return _priv_GLXRendererImpl::create((Display*)xDisplay, xWindow, param);
	}

}

#else

namespace slib
{

	Ref<Renderer> GLX::createRenderer(void* xDisplay, unsigned long xWindow, const RendererParam& param)
	{
		return sl_null;
	}

}

#endif
