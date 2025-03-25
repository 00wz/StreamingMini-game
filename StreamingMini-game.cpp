#include <string>
#include <thread>
#include <set>
#include <mutex>
#include <list>
#include <algorithm>
#include <iostream>
#include <random>
#include <conio.h>

const char border = '-';
const char car = 'D';
const char empty = ' ';
const int line_length = 60;

int delta_time_ms = 50;
/// <summary>
/// вероятность появления препятствия на пути
/// </summary>
double cars_spawn_probability_step = 0.05;
std::default_random_engine re;

/// <summary>
/// коллекции для хранения данных каждой полосы
/// </summary>
std::list<char> line0;
std::list<char> line1;
std::list<char> line2;
std::list<char> line3;
std::list<char> line4;

/// <summary>
/// положение игрока
/// </summary>
short player_pos = 1;

/// <summary>
/// мьютекс для огараничния доступа к положению игрока
/// </summary>
std::mutex player_pos_mutex;

/// <summary>
/// флаг конца инры
/// </summary>
std::atomic<bool> game_is_over;

/// <summary>
/// вызывается при начале игры и  заполняет все полосы
/// </summary>
static void Start()
{
    line0.assign(line_length, border);
    line1.assign(line_length, empty);
    line2.assign(line_length, empty);
    int i = 0;
    for(char& c : line2)
    {
        if (i++ % 4 == 0)
        {
            c = border;
        }
    }
    line3.assign(line_length, empty);
    line4.assign(line_length, border);
}

/// <summary>
/// основная логика игры происходит тут
/// </summary>
static void Update()
{
    //движение "разметки"
    line2.push_back(line2.front());
    line2.pop_front();

    //движение и появление препятствий
    auto get_next_fragment = []() 
        { 
            static double cars_spawn_probability = 0.0;
            std::uniform_real_distribution<double> unif(0.0, 1.0);
            if (unif(re) <= cars_spawn_probability)
            {
                //при появлении препятствий, вероятность их спавна уменьшается так, что бы они не появлялись как минимум 2 хода
                cars_spawn_probability = -cars_spawn_probability_step * 2;
                return car;
            }
                //чем дольше не появляются препятствия, тем больше вероятность их появления
            cars_spawn_probability += cars_spawn_probability_step;
            return empty;
        };
    line1.pop_front();
    line1.push_back(get_next_fragment());
    line3.pop_front();
    line3.push_back(get_next_fragment());

    //обработка столкновений
    std::unique_lock<std::mutex> lock{ player_pos_mutex };
    if (player_pos == 0)
    {
        if (line1.front() == car)
        {
            game_is_over = true;
        }
        line1.front() = car;
    }
    else
    {
        if (line3.front() == car)
        {
            game_is_over = true;
        }
        line3.front() = car;
    }
}

/// <summary>
/// отрисовка всех полос
/// </summary>
static void Draw()
{
    //очиска консоли
    system("cls");
    //вывод всех строк
    std::for_each(line0.begin(), line0.end(), [](const char& c) {std::cout << c;});
    std::cout << "\n";
    std::for_each(line1.begin(), line1.end(), [](const char& c) {std::cout << c;});
    std::cout << "\n";
    std::for_each(line2.begin(), line2.end(), [](const char& c) {std::cout << c;});
    std::cout << "\n";
    std::for_each(line3.begin(), line3.end(), [](const char& c) {std::cout << c;});
    std::cout << "\n";
    std::for_each(line4.begin(), line4.end(), [](const char& c) {std::cout << c;});
    std::cout << "\n";
}


/// <summary>
/// цикл обработки вода
/// </summary>
static void InputLoop()
{
    while (true) 
    {
        if (_kbhit()) // Проверка нажатия клавиши
        {
            _getch();
            std::unique_lock<std::mutex> lock{ player_pos_mutex };
            player_pos = player_pos == 0 ? 1 : 0;
        }
    }
}

/// <summary>
/// чередование игрового процесса и отрисовки
/// </summary>
static void GameLoop()
{
    game_is_over = false;
    while (!game_is_over)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(delta_time_ms));
        Update();
        Draw();
    }
}

int main()
{
    std::thread input_hundling(InputLoop);
    input_hundling.detach();

    //бесконечый рестарт после проигрыша
    while (true)
    {
        Start();
        std::thread game_loop(GameLoop);
        game_loop.join();
    }
}