#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"
#include "cinder/Log.h"

#include "AssetManager.h"
#include "MiniConfig.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class MultiDisplayTestApp : public App
{
  public:
    void setup() override
    {
        log::makeLogger<log::LoggerFile>();

        auto displays = Display::getDisplays();
        for (auto& display : displays)
        {
            auto window = getWindow();
            if (display != Display::getMainDisplay())
            {
                auto fmt = app::Window::Format().display(display);
                window = createWindow(fmt);
            }
            mWindows.emplace_back(window);
        }
        
        auto aabb = am::triMesh(MESH_NAME)->calcBoundingBox();
        mCam.lookAt(aabb.getMax() * 2.0f, aabb.getCenter());
        mCamUi = CameraUi( &mCam, getWindow(), -1 );
        
        createConfigUI({200, 200});
        gl::enableDepth();

        for (auto& window : mWindows)
        {
            window->getSignalDraw().connect([&] {
                gl::setMatrices(mCam);
                gl::clear();

                gl::ScopedTextureBind tex0(am::texture2d(TEX0_NAME), 0);
                gl::ScopedTextureBind tex1(am::texture2d(TEX1_NAME), 1);
                gl::ScopedTextureBind tex2(am::texture2d(TEX2_NAME), 2);
                gl::ScopedTextureBind tex3(am::texture2d(TEX3_NAME), 3);
                gl::ScopedGlslProg glsl(mGlslProg);

                gl::draw(am::vboMesh(MESH_NAME));
            });
        }

        getWindow()->getSignalResize().connect([&] {
            APP_WIDTH = getWindowWidth();
            APP_HEIGHT = getWindowHeight();
            mCam.setAspectRatio( getWindowAspectRatio() );
        });

        getSignalCleanup().connect([&] { writeConfig(); });

        getWindow()->getSignalKeyUp().connect([&](KeyEvent& event) {
            if (event.getCode() == KeyEvent::KEY_ESCAPE) quit();
        });
        
        mGlslProg = am::glslProg(VS_NAME, FS_NAME);
        mGlslProg->uniform("uTex0", 0);
        mGlslProg->uniform("uTex1", 1);
        mGlslProg->uniform("uTex2", 2);
        mGlslProg->uniform("uTex3", 3);

    }
    
private:
    CameraPersp         mCam;
    CameraUi            mCamUi;
    gl::GlslProgRef     mGlslProg;

    vector<app::WindowRef> mWindows;
};

CINDER_APP( MultiDisplayTestApp, RendererGl, [](App::Settings* settings) {
    readConfig();
    settings->setWindowSize(APP_WIDTH, APP_HEIGHT);
    settings->setMultiTouchEnabled(false);
} )
