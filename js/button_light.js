
(function () {
    const BUTTON = document.getElementById("butt");
    const BODY = document.body;

    if (!BUTTON) {
        console.error("Кнопка темы #butt не найдена");
        return;
    }

    let themeManager = null;
    let isAnimating = false;

    function getTheme() {
        return document.documentElement?.dataset.theme === "light"
            ? "light"
            : "dark";
    }

    function applyTheme(theme) {
        const root = document.documentElement;

        if (!root) {
            console.error("document.documentElement ещё не создан");
            return;
        }

        root.dataset.theme = theme;
        document.body?.classList.toggle("light", theme === "light");
    }

    // Применяем текущую тему без анимации.
    // При загрузке ROOT.dataset.theme уже должен быть установлен
    // ранним скриптом из C++.
    applyTheme(getTheme());

    window.appChannelReady
        ?.then(channel => {
            themeManager = channel.objects.themeManager;

            if (!themeManager) {
                console.error("ThemeManager не зарегистрирован в C++");
                return;
            }

            // Получаем сохранённую тему и применяем её без анимации.
            themeManager.currentTheme(function (savedTheme) {
                applyTheme(savedTheme === "light" ? "light" : "dark");
            });
        })
        .catch(error => {
            console.error("Ошибка подключения ThemeManager:", error);
        });

    BUTTON.addEventListener("click", function (e) {
        if (isAnimating) return;

        isAnimating = true;

        const currentTheme = getTheme();
        const nextTheme = currentTheme === "dark" ? "light" : "dark";
        const nextColor = nextTheme === "light" ? "#dcdcdc" : "#161616";

        const x = e.clientX;
        const y = e.clientY;

        const maxRadius = Math.ceil(Math.max(
            Math.hypot(x, y),
            Math.hypot(innerWidth - x, y),
            Math.hypot(x, innerHeight - y),
            Math.hypot(innerWidth - x, innerHeight - y)
        ));

        const pageRipple = document.createElement("div");
        pageRipple.className = "theme-ripple";
        pageRipple.style.left = `${x}px`;
        pageRipple.style.top = `${y}px`;
        pageRipple.style.backgroundColor = nextColor;
        BODY.appendChild(pageRipple);

        const rect = BUTTON.getBoundingClientRect();
        const buttonX = x - rect.left;
        const buttonY = y - rect.top;

        const buttonRipple = document.createElement("span");
        buttonRipple.className = "clicked";
        buttonRipple.style.left = `${buttonX}px`;
        buttonRipple.style.top = `${buttonY}px`;
        buttonRipple.style.backgroundColor = nextColor;
        BUTTON.appendChild(buttonRipple);

        const buttonRadius = Math.ceil(Math.max(
            Math.hypot(buttonX, buttonY),
            Math.hypot(rect.width - buttonX, buttonY),
            Math.hypot(buttonX, rect.height - buttonY),
            Math.hypot(rect.width - buttonX, rect.height - buttonY)
        ));

        // Запускаем CSS-переходы.
        pageRipple.offsetWidth;
        buttonRipple.offsetWidth;

        pageRipple.style.transform =
            `translate(-50%, -50%) scale(${maxRadius})`;

        buttonRipple.style.transform =
            `translate(-50%, -50%) scale(${buttonRadius})`;

        let finished = false;


        function finishAnimation() {
            if (finished) return;
            finished = true;

            const root = document.documentElement;

            // Отключаем плавные переходы перед сменой темы.
            root.classList.add("theme-switching");

            applyTheme(nextTheme);

            // Сохраняем выбранную тему.
            if (themeManager) {
                themeManager.setTheme(nextTheme);
            }

            // Даём браузеру применить цвета без переходов.
            requestAnimationFrame(() => {
                requestAnimationFrame(() => {
                    root.classList.remove("theme-switching");

                    pageRipple.remove();
                    buttonRipple.remove();

                    isAnimating = false;
                });
            });
        }

        pageRipple.addEventListener("transitionend", event => {
            if (event.target === pageRipple &&
                event.propertyName === "transform") {
                finishAnimation();
            }
        });

        // Защита от зависания, если transitionend не сработает.
        setTimeout(finishAnimation, 2000);
    });
})();
