#include <iostream>
#include <sstream>
#include <vector>
// SFML(socket, gui)
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
// OpenCV
#include <opencv2/opencv.hpp>
// モニター情報 & マウス操作
#ifdef __APPLE__
    #include <CoreGraphics/CoreGraphics.h>
    #include <ApplicationServices/ApplicationServices.h>
#endif



// Struct
struct screen_size
{
    float width;
    float height;
};

struct Finger_points
{
    float tx;
    float ty;
    float ix;
    float iy;
    float mx;
    float my;
    float rx;
    float ry;
    float px;
    float py;

    void x_align(float ratio)
    {
        tx *= ratio;
        ix *= ratio;
        mx *= ratio;
        rx *= ratio;
        px *= ratio;
    }

    void y_align(float ratio)
    {
        ty *= ratio;
        iy *= ratio;
        my *= ratio;
        ry *= ratio;
        py *= ratio;
    }
};



// Function
screen_size get_monitor_size()
{
    CGDirectDisplayID Display = CGMainDisplayID();
    CGDisplayModeRef mode = CGDisplayCopyDisplayMode(Display);
    float width = CGDisplayModeGetWidth(mode);
    float height = CGDisplayModeGetHeight(mode);
    CGDisplayModeRelease(mode);
    return {width, height};
}

void MouseMove(float x, float y)
{
    CGEventRef move = CGEventCreateMouseEvent(NULL, kCGEventMouseMoved, CGPointMake(x,y),kCGMouseButtonLeft);
    CGEventPost (kCGHIDEventTap, move);
    CFRelease(move);
}




int main()
{
  
    screen_size cam_size;
    screen_size monitor_size = get_monitor_size();
    
    // socket宣言および5005ポートに接続
    sf::UdpSocket socket;
    if (socket.bind(5005) != sf::Socket::Status::Done) return -1;
    
    Finger_points finger_points;
    std::size_t received;
    std::optional<sf::IpAddress> sender;
    unsigned short port;

    // get cam_size
    if (socket.receive(&cam_size, sizeof(cam_size), received, sender, port) == sf::Socket::Status::Done)
    {
        std::cout << cam_size.width << " " << cam_size.height << std::endl;
    }
    socket.setBlocking(false);
    std::cout << monitor_size.width << " " << monitor_size.height << std::endl;



    sf::RenderWindow window(sf::VideoMode({800,600}), "title");
    sf::CircleShape thumb(10.f);
    sf::CircleShape index(10.f);
    sf::CircleShape middle(10.f);
    sf::CircleShape ring(10.f);
    sf::CircleShape pinky(10.f);
    
    


    // main loop
    while (window.isOpen())
    {
        if (socket.receive(&finger_points, sizeof(finger_points), received, sender, port) == sf::Socket::Status::Done)
        {
            finger_points.x_align(monitor_size.width);
            finger_points.y_align(monitor_size.height);
            MouseMove(finger_points.ix, finger_points.iy);
        }


        // SFML イベント
        while (std::optional event = window.pollEvent())
        {
            // close
            if (event->is<sf::Event::Closed>()) window.close();
            // resize
            else if(const auto* resized = event->getIf<sf::Event::Resized>())
            {
                sf::View view(sf::FloatRect({0.f, 0.f}, sf::Vector2f(window.getSize())));
                window.setView(view);
                finger_points.x_align(static_cast<float>(window.getSize().x) / cam_size.width);
                finger_points.y_align(static_cast<float>(window.getSize().y) / cam_size.height);
            }
        }




        // SFML 描画
        window.clear(sf::Color::Black);
        thumb.setPosition({finger_points.tx,finger_points.ty});
        index.setPosition({finger_points.ix,finger_points.iy});
        middle.setPosition({finger_points.mx,finger_points.my});
        ring.setPosition({finger_points.rx,finger_points.ry});
        pinky.setPosition({finger_points.px,finger_points.py});

        window.draw(thumb);
        window.draw(index);
        window.draw(middle);
        window.draw(ring);
        window.draw(pinky);

        window.display();
    }


    return 0;
}