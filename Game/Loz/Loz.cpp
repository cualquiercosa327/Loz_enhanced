/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "Graphics.h"
#include "Input.h"
#include "Sound.h"
#include "World.h"
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_image.h>


const double FrameTime = 1 / 60.0;


static ALLEGRO_EVENT_QUEUE* eventQ;
static ALLEGRO_DISPLAY* display;
static uint32_t frameCounter;


void ResizeView( int screenWidth, int screenHeight );

uint32_t GetFrameCounter()
{
    return frameCounter;
}

void Run()
{
    ALLEGRO_EVENT event = { 0 };
    ALLEGRO_EVENT_SOURCE* keyboardSource = al_get_keyboard_event_source();
    ALLEGRO_EVENT_SOURCE* displaySource = al_get_display_event_source( display );

    if ( keyboardSource == nullptr )
        return;
    if ( displaySource == nullptr )
        return;

    al_register_event_source( eventQ, keyboardSource );
    al_register_event_source( eventQ, displaySource );

    World world;

    world.Init();

    double startTime = al_get_time();

    while ( true )
    {
        bool updated = false;

        if ( !al_is_event_queue_empty( eventQ ) )
        {
            al_wait_for_event( eventQ, &event );
            if ( event.any.type == ALLEGRO_EVENT_DISPLAY_CLOSE
                || (event.any.type == ALLEGRO_EVENT_KEY_DOWN 
                && event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) )
                break;
            else if ( event.any.type == ALLEGRO_EVENT_DISPLAY_RESIZE )
            {
                al_acknowledge_resize( display );

                ResizeView( event.display.width, event.display.height );

                updated = true;
            }
        }

        double now = al_get_time();

        while ( (now - startTime) >= FrameTime )
        {
            frameCounter++;

            Input::Update();
            world.Update();
            Sound::Update();

            startTime += FrameTime;
            updated = true;
        }

        if ( updated )
        {
            world.Draw();

            al_flip_display();
        }
    }
}

void ResizeView( int screenWidth, int screenHeight )
{
    float viewAspect = StdViewWidth / (float) StdViewHeight;
    float screenAspect = screenWidth / (float) screenHeight;
    // Only allow whole number scaling
    int scale = 1;

    if ( viewAspect > screenAspect )
    {
        scale = screenWidth / StdViewWidth;
    }
    else
    {
        scale = screenHeight / StdViewHeight;
    }

    if ( scale <= 0 )
        scale = 1;

    int viewWidth = StdViewWidth * scale;
    int viewHeight = StdViewHeight * scale;

    // It looks better when the offsets are whole numbers
    int offsetX = (screenWidth - viewWidth) / 2;
    int offsetY = (screenHeight - viewHeight) / 2;

    al_set_clipping_rectangle( offsetX, offsetY, viewWidth, viewHeight );

    ALLEGRO_TRANSFORM t;

    al_identity_transform( &t );
    al_scale_transform( &t, scale, scale );
    al_translate_transform( &t, offsetX, offsetY );
    al_use_transform( &t );

    Graphics::SetViewParams( scale, offsetX, offsetY );
}

bool MakeDisplay()
{
    int width = StdViewWidth;
    int height = StdViewHeight;
    int newFlags = ALLEGRO_RESIZABLE | ALLEGRO_PROGRAMMABLE_PIPELINE;

    newFlags |= ALLEGRO_DIRECT3D_INTERNAL;

    al_set_new_display_flags( al_get_new_display_flags() | newFlags );

    display = al_create_display( width, height );
    if ( display == nullptr )
        return false;

    ResizeView( width, height );

    return true;
}

bool InitAllegro()
{
    if ( !al_init() )
        return false;

    if ( !al_install_keyboard() )
        return false;

    if ( !al_init_image_addon() )
        return false;

    if ( !al_install_audio() )
        return false;

    if ( !al_init_acodec_addon() )
        return false;

    if ( !MakeDisplay() )
        return false;

    eventQ = al_create_event_queue();
    if ( eventQ == nullptr )
        return false;

    if ( !Graphics::Init() )
        return false;

    if ( !Sound::Init() )
        return false;

    return true;
}

int main( int argc, char* argv[] )
{
    if ( InitAllegro() )
    {
        Run();
    }

    return 0;
}
