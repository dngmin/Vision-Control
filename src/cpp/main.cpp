#include <iostream>
#include <sstream>
#include <array>
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
    // Kalman Filterで手ブレ補正

    // 初期化
    // 状態予測値
    sf::Vector2f Predict;
    // F；運動モデル：ランダムウォークモデル、H：観測行列
    std::array<std::array<int, 2>, 2> F, H;
    F = {{{1, 0}, {0, 1}}};
    H = {{{1, 0}, {0, 1}}};
    // P；誤差共分散、Q：プロセスノイズ、R：センサーノイズ、K：Kalman Gain
    float P = 1.f, Q = 1e-5, R = 1e-4, K;
  
    screen_size cam_size;
    screen_size monitor_size = get_monitor_size();
    
    // socket宣言および5005ポートに接続
    sf::UdpSocket socket;
    if (socket.bind(5005) != sf::Socket::Status::Done) return -1;
    
    Finger_points finger_points;
    std::size_t received;
    std::optional<sf::IpAddress> sender;
    unsigned short port;

    // cam_size取得
    if (socket.receive(&cam_size, sizeof(cam_size), received, sender, port) == sf::Socket::Status::Done)
    {
        std::cout << cam_size.width << " " << cam_size.height << std::endl;
    }
    // finger_pointss初期化
    if (socket.receive(&finger_points, sizeof(finger_points), received, sender, port) == sf::Socket::Status::Done)
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
        // Kalman Filterの実装
        // ランダムウォークモデルであるため、予測値＝前回の観測値
        // 予測
        Predict.x = finger_points.ix;
        Predict.y = finger_points.iy;
        if (socket.receive(&finger_points, sizeof(finger_points), received, sender, port) == sf::Socket::Status::Done)
        {
            // 誤差共分散予測
            P += Q;

            // 観測
            finger_points.x_align(monitor_size.width);
            finger_points.y_align(monitor_size.height);

            // Kalman Gain計算
            K = P / (P + R);

            // 状態更新
            finger_points.ix = Predict.x + K*(finger_points.ix - Predict.x);
            finger_points.iy = Predict.y + K*(finger_points.iy - Predict.y);

            // 共分散更新
            P = (1.f - K)*P;


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