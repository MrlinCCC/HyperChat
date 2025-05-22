#include <iostream>
#include <Asio.hpp>

int testAiso()
{
    try
    {
        asio::io_context io_context;

        // 解析域名和服务（HTTP默认端口80）
        asio::ip::tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("www.baidu.com", "80");

        // 创建socket并连接
        asio::ip::tcp::socket socket(io_context);
        asio::connect(socket, endpoints);

        // 发送 HTTP GET 请求
        const std::string request =
            "GET / HTTP/1.1\r\n"
            "Host: www.baidu.com\r\n"
            "Connection: close\r\n\r\n";
        asio::write(socket, asio::buffer(request));

        // 接收响应
        std::array<char, 4096> buffer;
        asio::error_code error;

        while (true)
        {
            size_t len = socket.read_some(asio::buffer(buffer), error);
            if (error == asio::error::eof)
            {
                // 连接关闭，数据读取完毕
                break;
            }
            else if (error)
            {
                throw asio::system_error(error);
            }
            std::cout << "Asio read: " << len << "\n";
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}

int main(int argc, char *argv[])
{
    testAiso();
    std::cin.get();
    return 0;
}