#include "Common.hpp"
#include "Fullscreen/Fullscreen.hpp"

constexpr int32 TimeLimit = 15000; // 制限時間は15秒

void Init() {
    System::SetTerminationTriggers(UserAction::CloseButtonClicked);

    Window::SetTitle(U"PrimeFactorize");
    Window::Resize(1280, 720);
    Window::SetStyle(WindowStyle::Sizable);
    Scene::SetBackground(Palette::Darkgray);

    Fullscreen::Init(true);
}

void Finalize() {
    
}

class Title : public MyApp::Scene {
public:

    Title(const InitData& init)
        : IScene(init) {}

    void update() override {
        m_startTransition.update(m_startButton.mouseOver());
        m_exitTransition.update(m_exitButton.mouseOver());

        if (m_startButton.mouseOver() || m_exitButton.mouseOver()) {
            Cursor::RequestStyle(CursorStyle::Hand);
        }

        if (m_startButton.leftClicked()) {
            changeScene(SceneState::Game);

        }

        if (m_exitButton.leftClicked()) {
            System::Exit();
        }
    }

    void draw() const override {
        const String titleText = U"Prime Factorize";
        const Vec2 center(Scene::Center().x, 120);
        FontAsset(U"Title")(titleText).drawAt(center.movedBy(4, 6), ColorF(0.0, 0.5));
        FontAsset(U"Title")(titleText).drawAt(center);
        FontAsset(U"Menu")(U"素因数分解ゲーム").drawAt(center.movedBy(0, 120), ColorF(1.0, 0.7));


        m_startButton.draw(ColorF(0.5, m_startTransition.value())).drawFrame(2);
        m_exitButton.draw(ColorF(0.5, m_exitTransition.value())).drawFrame(2);

        FontAsset(U"Menu")(U"はじめる").drawAt(m_startButton.center(), ColorF(1.0));
        FontAsset(U"Menu")(U"おわる").drawAt(m_exitButton.center(), ColorF(1.0));

        Rect(0, 500, Scene::Width(), Scene::Height() - 500)
            .draw(Arg::top = ColorF(0.0, 0.0), Arg::bottom = ColorF(0.0, 0.5));

        const int32 highScore = getData().highScore;
        FontAsset(U"Score")(U"High score: {}"_fmt(highScore)).drawAt(Vec2(620, 550));
    }

private:

    Rect m_startButton = Rect(Arg::center = Scene::Center().movedBy(0, 0), 300, 60);
    Transition m_startTransition = Transition(0.4s, 0.2s);

    Rect m_exitButton = Rect(Arg::center = Scene::Center().movedBy(0, 100), 300, 60);
    Transition m_exitTransition = Transition(0.4s, 0.2s);

};

class Game : public MyApp::Scene {

public:

    Game(const InitData& init)
        : IScene(init), m_stopwatch(true), m_gameCount(true), m_backToTitle(Size(100, 100)) {

        for (auto p : step(m_primes.size())) {
            m_buttons << Rect(Scene::Width() / m_primes.size() * p, Scene::Height() - 200, Scene::Width() / m_primes.size(), 200);
        }

        generateNumber();

        AudioAsset(U"BGM").setLoop(true);
        AudioAsset(U"BGM").play();


    }

    void update() override {

        if (!m_isGameover) {

            if (m_number == 1) {
                AudioAsset(U"AC").playOneShot();

                nextRound();
                return;
            }

            if (m_stopwatch.ms() > TimeLimit) {
                m_isGameover = true;
                AudioAsset(U"WA").playOneShot();
                AudioAsset(U"BGM").stop();
                return;
            }

            int32 index = 0;
            for (const auto& button : m_buttons) {
                if (button.stretched(-4).leftClicked()) {
                    if (m_number % m_primes[index] == 0) {
                        m_number /= m_primes[index];
                        AudioAsset(U"Click").playOneShot();
                    }
                    else {
                        m_isGameover = true;
                        AudioAsset(U"WA").playOneShot();
                        AudioAsset(U"BGM").stop();
                        break;
                    }
                }

                index++;
            }
        }
        else {
            if (m_backToTitle.movedBy(Scene::Width() - 120, Scene::Height() - 120).leftClicked()) {
                changeScene(SceneState::Title);
            }
        }
    }

    void draw() const override {
        Rect(Scene::Size()).draw(Palette::Black);

        if (!m_isGameover) {
            RectF(0, 0, Scene::Width() * ((double)(TimeLimit - m_stopwatch.ms()) / TimeLimit), 10).draw(ColorF(Palette::Red, 0.7));

            if (m_gameCount.ms() < m_notification.first) {
                FontAsset(U"Game.button")(U"{}"_fmt(m_notification.second)).draw(30, 20);
            }

            FontAsset(U"Game")(U"{}"_fmt(m_number)).drawAt(Scene::Center().x, 240);

            FontAsset(U"Score")(U"スコア : {}"_fmt(m_score)).draw(Arg::center = Vec2(Scene::Width() - 200, 30));


            int32 index = 0;
            for (const auto& button : m_buttons) {
                button.stretched(-4).drawShadow({ 0.0, 0.0 }, 20, 2, Palette::White).stretched(-10).draw(ColorF(0.7));
                FontAsset(U"Game.button")(U"÷{}"_fmt(m_primes[index])).draw(Arg::center = button.center());

                index++;
            }
        }
        else {
            FontAsset(U"Game")(U"GAME OVER!").drawAt(Scene::Center().x, Scene::Center().y - 100, Palette::Red);

            FontAsset(U"Game.button")(U"スコア : {}"_fmt(m_score)).drawAt(Scene::Center().x, Scene::Center().y + 100);

            m_backToTitle.movedBy(Scene::Width() - 120, Scene::Height() - 120).drawFrame(2.0);
            FontAsset(U"Score")(U"戻る").draw(Arg::center = m_backToTitle.movedBy(Scene::Width() - 120, Scene::Height() - 120).center());


        }
    }

private:

    void nextRound() {
        m_stopwatch.restart();
        m_score++;

        if (m_score % 5 == 0) {
            m_level++;
            m_notification = { m_gameCount.ms() + 3000, U"Level UP!"};

            if (m_level == 5) {
                m_primes.push_back(11);
            }
            if (m_level == 7) {
                m_primes.push_back(13);
            }

            m_buttons.clear();
            for (auto p : step(m_primes.size())) {
                m_buttons << Rect(Scene::Width() / m_primes.size() * p, Scene::Height() - 200, Scene::Width() / m_primes.size(), 200);
            }
        }

        if (getData().highScore < m_score) {
            getData().highScore = m_score;
        }

        generateNumber();
    }

    void generateNumber() {
        m_number = 1;

        for (int32 i = 0; i < m_level + 1; i++) {
            m_number *= m_primes[Random(m_primes.size() - 1)];
        }
    }


    Stopwatch m_stopwatch;
    Stopwatch m_gameCount;
    int32 m_score = 0;
    int64 m_number = 1;
    int32 m_level = 1;
    Array<int32> m_primes = { 2, 3, 5, 7 /*, 11, 13 */};
    Array<Rect> m_buttons;
    bool m_isGameover = false;
    int32 m_blank = -100;
    Rect m_backToTitle;
    std::pair<int32, String> m_notification;
};


void Main() {

    FontAsset::Register(U"Title", 120, U"example/font/AnnyantRoman/AnnyantRoman.ttf");
    FontAsset::Register(U"Menu", 30, Typeface::Regular);
    FontAsset::Register(U"Game", 120, Typeface::Black);
    FontAsset::Register(U"Game.button", 70, Typeface::Black);
    FontAsset::Register(U"Score", 36, Typeface::Bold);

    AudioAsset::Register(U"Click", U"dat/clicksound.wav");
    AudioAsset::Register(U"WA", U"dat/wa.mp3");
    AudioAsset::Register(U"AC", U"dat/ac.mp3");
    AudioAsset::Register(U"BGM", U"dat/bgm.mp3");

    Init();

    MyApp manager;
    manager
        .add<Title>(SceneState::Title)
        .add<Game>(SceneState::Game)
        .setFadeColor(ColorF(1.0));
    manager.init(SceneState::Title);

    while (System::Update() && manager.update()) {

        if (KeyF11.down()) Fullscreen::Toggle();

    }

    Finalize();

}