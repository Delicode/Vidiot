#include "VideoView.h"
#include <QOpenGLFunctions>
#include <QDebug>
#include <QPainter>
#include <QOpenGLFramebufferObjectFormat>
#include "VideoPlayer.h"
#include <QOpenGLShaderProgram>

#define GLER {GLenum er = glGetError(); if(er != GL_NO_ERROR) qDebug() << "GLerror" << __FILE__ << ", " << __LINE__ << er;}


char glsl_color[] = {
    "#version 150\n"
    "uniform sampler2D tex;\n"
    "in vec2 TexCoord;\n"
    "out vec4 Out;\n"

    "void main()\n"
    "{\n"
        "Out = texture(tex, TexCoord);\n"
        "Out.a = 1.0;\n"
    "}\n"
};

char glsl_quad_vert[] = {
    "#version 150\n"
    "in vec2 vertex;\n"
    "in vec2 tcoord;\n"
    "out vec2 TexCoord;\n"
    "uniform vec2 scale;\n"
    "void main( void ) {\n"
    "   gl_Position = vec4(scale*vertex, 0.0, 1.0);\n"
    "   TexCoord = tcoord;\n"
    "}"
};



bool FboDrawer::ensureBuffers()
{
    if(init_glbuffers) {
        init_glbuffers = false;

        if(!verts.isCreated()) {
            GLER
            float quad_verts[] = {
                -1.f, 1.f,
                1.f, 1.f,
                -1.f, -1.f,
                1.f, -1.f
            };

            if (!verts.create()) {
                GLER
                qDebug() << "failed creating vert buffer";
                return false;
            }
            GLER
            verts.setUsagePattern(QOpenGLBuffer::StaticDraw);
            GLER
            if (!verts.bind()) {
                GLER
                qDebug() << "failed binding verts buffer";
                return false;
            }
            GLER
            verts.allocate(quad_verts, 2*4*sizeof(float));
            GLER
            verts.release();
            GLER
        }

        if(!tcoords.isCreated()) {

            float quad_tcoords[] = {
                0.f, 0.f,
                1.f, 0.f,
                0.f, 1.f,
                1.f, 1.f
            };
            if (!tcoords.create()){
                qDebug() << "failed creating tcoords buffer";
                return false;
            }
            tcoords.setUsagePattern(QOpenGLBuffer::StaticDraw);
            if (!tcoords.bind()) {
                qDebug() << "failed binding tcoords buffer";
                return false;
            }
            tcoords.allocate(quad_tcoords, 2*4*sizeof(float));
            tcoords.release();
        }


        glfunc->glGenVertexArrays(1, &vaoQuad);
        GLER
        glfunc->glBindVertexArray(vaoQuad);
        GLER
        verts.bind();
        GLER
            glfunc->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
        GLER
            glfunc->glEnableVertexAttribArray(0);
        GLER
        tcoords.bind();
        GLER
            glfunc->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
        GLER
            glfunc->glEnableVertexAttribArray(1);
        GLER
        glfunc->glBindVertexArray(0);
        GLER

    }

    return true;
}

void FboDrawer::drawFBO(QOpenGLFramebufferObject *fbo, QSize screen)
{
    static bool gl_error_reported = false;

    if(!screen_prog)
    {
        screen_prog = new QOpenGLShaderProgram();
    }

    GLenum glerr = glGetError();
    if(glerr != GL_NO_ERROR) {
        if(!gl_error_reported)
            qDebug() << "Error while trying to draw live view: " + QString::number(glerr);
        //return;
    }

    gl_error_reported = false;

    if (!ensureBuffers()) {
        qDebug() << "failed ensuring buffers";
        return;
    }

    GLER

    if(!screen_prog->isLinked()) {
        qDebug() << QString("Linking ") + screen_prog->objectName();
        screen_prog->removeAllShaders();

        QString frag = QString(glsl_color);
        if(!screen_prog->addShaderFromSourceCode(QOpenGLShader::Fragment, frag))
            qDebug() << screen_prog->objectName() + " fragment compile: " + screen_prog->log();

        QString vert = QString(glsl_quad_vert);
        if(!screen_prog->addShaderFromSourceCode(QOpenGLShader::Vertex, vert))
            qDebug() << screen_prog->objectName() + " vertex compile: " + screen_prog->log();

        screen_prog->bindAttributeLocation("vertex", 0);
        screen_prog->bindAttributeLocation("tcoord", 1);
        if(!screen_prog->link())
            qDebug() << screen_prog->objectName() + " link: " + screen_prog->log();
    }

    GLER

    if(!screen_prog->isLinked())
        return;

    GLER
    glfunc->glBindVertexArray(vaoQuad);
    GLER
    QOpenGLContext::currentContext()->functions()->glBindTexture(GL_TEXTURE_2D, fbo->texture());

    float fbo_aspect = (float)fbo->width()/(float)fbo->height();
    float screen_aspect = (float)screen.width()/(float)screen.height();
    float scalex = 1;
    float scaley = 1;

    if(fbo_aspect < screen_aspect)
        scalex = fbo_aspect/screen_aspect;
    else
        scaley = screen_aspect/fbo_aspect;

    if(screen_prog->bind()) {
        screen_prog->setUniformValue("tex", 0);
        screen_prog->setUniformValue("scale", QVector2D(scalex, scaley));

        QOpenGLContext::currentContext()->functions()->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        screen_prog->release();
    }
    GLER
    glfunc->glBindVertexArray(0);

    GLER
}

void FboDrawer::drawTexture(unsigned int texid, int width, int height, bool flip, QSize screen)
{
    static bool gl_error_reported = false;

    if(!screen_prog)
        screen_prog = new QOpenGLShaderProgram();

    GLenum glerr = glGetError();
    if(glerr != GL_NO_ERROR) {
        if(!gl_error_reported)
            qDebug() << "Error while trying to draw live view: " + QString::number(glerr);
        //return;
    }

    gl_error_reported = false;

    if (!ensureBuffers()) {
        qDebug() << "failed ensuring buffers";
        return;
    }

    GLER

    if(!screen_prog->isLinked()) {
        qDebug() << QString("Linking ") + screen_prog->objectName();
        screen_prog->removeAllShaders();

        QString frag = QString(glsl_color);
        if(!screen_prog->addShaderFromSourceCode(QOpenGLShader::Fragment, frag))
            qDebug() << screen_prog->objectName() + " fragment compile: " + screen_prog->log();

        QString vert = QString(glsl_quad_vert);
        if(!screen_prog->addShaderFromSourceCode(QOpenGLShader::Vertex, vert))
            qDebug() << screen_prog->objectName() + " vertex compile: " + screen_prog->log();

        screen_prog->bindAttributeLocation("vertex", 0);
        screen_prog->bindAttributeLocation("tcoord", 1);
        if(!screen_prog->link())
            qDebug() << screen_prog->objectName() + " link: " + screen_prog->log();
    }

    GLER

    if(!screen_prog->isLinked())
        return;

    GLER

    glfunc->glBindVertexArray(vaoQuad);
    GLER

    QOpenGLContext::currentContext()->functions()->glBindTexture(GL_TEXTURE_2D, texid);

    float fbo_aspect = (float)width/(float)height;
    float screen_aspect = (float)screen.width()/(float)screen.height();
    float scalex = 1;
    float scaley = 1;

    if(fbo_aspect < screen_aspect)
        scalex = fbo_aspect/screen_aspect;
    else
        scaley = screen_aspect/fbo_aspect;

    if(flip == false)
        scaley *= -1.f;

    scalex = 1.f;
    scaley = 1.f;

    if(screen_prog->bind()) {
        screen_prog->setUniformValue("tex", 0);
        screen_prog->setUniformValue("scale", QVector2D(scalex, scaley));
        QOpenGLContext::currentContext()->functions()->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        screen_prog->release();
    }

    GLER

    glfunc->glBindVertexArray(0);

    GLER
}

class TextureInFboRenderer : public QQuickFramebufferObject::Renderer
{
public:
    TextureInFboRenderer() {
        texid = 0;
        width = 0;
        height = 0;
        flip = true;
    }

    void render() {
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        GLenum er = glGetError();
        if(er != GL_NO_ERROR)
            qDebug() << "GLerror1" << er;

        drawer.drawTexture(texid, width, height, flip, framebufferObject()->size());
    }

    void synchronize(QQuickFramebufferObject *item) {
        VideoView *vv = static_cast<VideoView *>(item);
        texid = vv->texid;
        width = vv->texwidth;
        height = vv->texheight;
        flip = vv->texflip;
    }

    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) {
        QOpenGLFramebufferObjectFormat fmt;
        return new QOpenGLFramebufferObject(size, fmt);
    }

protected:

private:
    FboDrawer drawer;
    unsigned int texid;
    int width, height;
    bool flip;
};

QQuickFramebufferObject::Renderer *VideoView::createRenderer() const
{
    return new TextureInFboRenderer();
}
