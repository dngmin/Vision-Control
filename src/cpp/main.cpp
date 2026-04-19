#include <iostream>
#include <sstream>
#include <vector>
// SFML(socket, gui)
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
// OpenCV
#include <opencv2/opencv.hpp>
// モニター情報
#ifdef __APPLE__
    #include <CoreGraphics/CoreGraphics.h>
#endif
// マウス制御

std::vector<int> get_monitor_size()
{
    CGDirectDisplayID Display = CGMainDisplayID();
    CGDisplayModeRef mode = CGDisplayCopyDisplayMode(Display);
    int width = CGDisplayModeGetPixelWidth(mode);
    int height = CGDisplayModeGetPixelHeight(mode);
    CGDisplayModeRelease(mode);
    return {width, height};
}

std::vector<int> get_cam()
{
    cv::VideoCapture cap(0);
    return {
        static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH)),
        static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT))
    };
}

const std::vector<int> monitor_size = get_monitor_size();
const std::vector<int> web_size = get_cam();









int main()
{
  
    std::cout << monitor_size[0] << " " << monitor_size[1] << std::endl;
    std::cout << web_size[0] << " " << web_size[1] << std::endl;
    get_monitor_size();
    sf::UdpSocket socket;

    if (socket.bind(5005) != sf::Socket::Status::Done) return -1;
    socket.setBlocking(false);
    char data[1024];
    std::size_t received;
    std::optional<sf::IpAddress> sender;
    unsigned short port;



    sf::RenderWindow window(sf::VideoMode({800,600}), "title");
    sf::CircleShape thumb(10.f);
    sf::CircleShape index(10.f);
    sf::CircleShape middle(10.f);
    sf::CircleShape ring(10.f);
    sf::CircleShape pinky(10.f);
    float tx,ty, ix,iy, mx,my, rx,ry, px,py;
    


    // main loop
    while (window.isOpen())
    {
        if (socket.receive(data, sizeof(data), received, sender, port) == sf::Socket::Status::Done)
        {
            std::string message(data,received);
            std::stringstream coords(message);
            coords >> tx >> ty >> ix >> iy >> mx >> my >> rx >> ry >> px >> py;
        }
        




        // SFML 描画
        while (std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>()) window.close();
        }
        window.clear(sf::Color::Black);
        thumb.setPosition({tx,ty});
        index.setPosition({ix,iy});
        middle.setPosition({mx,my});
        ring.setPosition({rx,ry});
        pinky.setPosition({px,py});

        window.draw(thumb);
        window.draw(index);
        window.draw(middle);
        window.draw(ring);
        window.draw(pinky);

        window.display();


    }
    return 0;
}

// g++ -std=c++20 src/cpp/main.cpp -o main $(pkg-config --cflags --libs sfml-all)