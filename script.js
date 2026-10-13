const girls = ['1.webp', '2.webp', '3.webp'];
const slider = document.getElementById('slider');

// 1. Заполняем слайдер и СРАЗУ расставляем их в ряд: 0%, 100%, 200%
girls.forEach((src, index) => {
    const img = document.createElement('img');
    img.src = src;
    // Задаем начальное положение в процентах
    img.style.transform = `translateX(${index * 100}%)`;
    slider.appendChild(img);
});

let isAnimating = false;

function myCommand() {
    if (isAnimating) return; // Защита от спам-кликов во время анимации
    isAnimating = true;

    const imgs = slider.querySelectorAll('img');

    // 2. Сначала включаем плавность и двигаем ВСЕ картинки влево на 100%
    imgs.forEach(img => {
        // Добавляем плавность перед движением
        img.style.transition = 'transform 0.5s ease';

        // Получаем текущий X (извлекаем число из строки "translateX(100%)")
        const currentX = parseInt(img.style.transform.replace('translateX(', '')) || 0;

        // Сдвигаем влево на ширину экрана (-100%)
        img.style.transform = `translateX(${currentX - 100}%)`;
    });

    // 3. Ждем окончания анимации (500мс)
    setTimeout(() => {
        imgs.forEach(img => {
            const currentX = parseInt(img.style.transform.replace('translateX(', '')) || 0;

            // Если картинка улетела влево (её позиция стала -100%)
            if (currentX < 0) {
                // МГНОВЕННО убираем плавность, чтобы пользователь не видел обратную перемотку
                img.style.transition = 'none';
                // Переносим её в самый конец (на 200%, так как картинок всего 3)
                img.style.transform = 'translateX(200%)';
            }
        });

        isAnimating = false; // Разрешаем следующий шаг
    }, 500); // Время совпадает с 0.5s анимации
}


setInterval(myCommand, 4000);


