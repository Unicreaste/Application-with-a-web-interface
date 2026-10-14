
const colors = [
    "#dcdcdc", // Светлая тема
    "#161616"  // Тёмная тема
];

let themeIndex = 1;
let isAnimating = false;

const BUTTON = document.getElementById("butt");
const BODY = document.body;

BODY.style.backgroundColor = colors[1];
BUTTON.style.backgroundColor = colors[1];
BUTTON.style.color = "#fff";

BUTTON.addEventListener("click", (e) => {
    if (isAnimating) return;

    isAnimating = true;

    const nextIndex = themeIndex === 1 ? 0 : 1;
    const nextColor = colors[nextIndex];

    // Координаты клика относительно окна
    const x = e.clientX;
    const y = e.clientY;

    // Максимальный радиус для покрытия всего экрана
    const maxRadius = Math.ceil(Math.max(
        Math.hypot(x, y),
        Math.hypot(innerWidth - x, y),
        Math.hypot(x, innerHeight - y),
        Math.hypot(innerWidth - x, innerHeight - y)
    ));

    // 1. Создаём круг для всей страницы
    const pageRipple = document.createElement("div");
    pageRipple.className = "theme-ripple";
    pageRipple.style.left = `${x}px`;
    pageRipple.style.top = `${y}px`;
    pageRipple.style.backgroundColor = nextColor;

    BODY.appendChild(pageRipple);

    // 2. Находим координаты клика внутри кнопки
    const rect = BUTTON.getBoundingClientRect();
    const buttonX = x - rect.left;
    const buttonY = y - rect.top;

    // 3. Создаём круг внутри кнопки
    const buttonRipple = document.createElement("span");
    buttonRipple.className = "clicked";
    buttonRipple.style.left = `${buttonX}px`;
    buttonRipple.style.top = `${buttonY}px`;
    buttonRipple.style.backgroundColor = nextColor;

    BUTTON.appendChild(buttonRipple);

    // Рассчитываем радиус, достаточный для покрытия кнопки
    const buttonRadius = Math.ceil(Math.max(
        Math.hypot(buttonX, buttonY),
        Math.hypot(rect.width - buttonX, buttonY),
        Math.hypot(buttonX, rect.height - buttonY),
        Math.hypot(rect.width - buttonX, rect.height - buttonY)
    ));

    // Фиксируем начальное состояние обоих кругов
    pageRipple.offsetWidth;
    buttonRipple.offsetWidth;

    // 4. Запускаем обе анимации одновременно
    pageRipple.style.transform =
        `translate(-50%, -50%) scale(${maxRadius})`;

    buttonRipple.style.transform =
        `translate(-50%, -50%) scale(${buttonRadius})`;

    // 5. После завершения перехода фиксируем новую тему

    pageRipple.addEventListener("transitionend", (event) => {
        if (event.propertyName !== "transform") return;

        // Сначала закрепляем новый фон под кругом
        BODY.classList.toggle("light", nextIndex === 0);
        BODY.style.backgroundColor = nextColor;

        BUTTON.style.backgroundColor = nextColor;
        BUTTON.style.color = nextIndex === 0 ? "#000000ff" : "#fff";

        BUTTON.style.boxShadow = nextIndex === 0
            ? "0 1rem 2rem rgba(0, 0, 0, 0.5)"
            : "0 1rem 2rem rgba(255, 255, 255, 0.5)";

        themeIndex = nextIndex;

        // Удаляем круги после фиксации фона
        pageRipple.remove();
        buttonRipple.remove();

        isAnimating = false;
    }, { once: true });

});
