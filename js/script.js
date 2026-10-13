const girls = [
    'images/1.webp',
    'images/2.webp',
    'images/3.webp'
];

const slider = document.getElementById('slider');

let isAnimating = false;

// Создаём все изображения заранее.
girls.forEach((src, index) => {
    const img = document.createElement('img');

    img.src = src;
    img.loading = 'eager';
    img.style.transform = `translateX(${index * 100}%)`;

    slider.appendChild(img);
});

async function startSlider() {
    const imgs = Array.from(slider.querySelectorAll('img'));

    // Ждём загрузки и декодирования всех изображений.
    await Promise.all(
        imgs.map(async (img) => {
            try {
                if (!img.complete) {
                    await new Promise((resolve, reject) => {
                        img.addEventListener('load', resolve, { once: true });
                        img.addEventListener('error', reject, { once: true });
                    });
                }

                if (img.naturalWidth > 0) {
                    await img.decode();
                }
            } catch (error) {
                console.error('Не удалось подготовить изображение:', img.src, error);
            }
        })
    );

    // Только теперь начинаем переключение.
    setTimeout(() => {
        requestAnimationFrame(() => {
            requestAnimationFrame(() => {
                setInterval(myCommand, 4000);
            });
        });
    }, 500);
}

function myCommand() {
    if (isAnimating) return;

    isAnimating = true;

    const imgs = Array.from(slider.querySelectorAll('img'));

    // Сначала включаем transition, не меняя положения картинок.
    imgs.forEach(img => {
        img.style.transition = 'transform 0.5s ease';
    });

    // Даём браузеру применить начальные стили до движения.
    requestAnimationFrame(() => {
        requestAnimationFrame(() => {
            imgs.forEach(img => {
                const currentX =
                    parseInt(img.style.transform.replace('translateX(', '')) || 0;

                img.style.transform = `translateX(${currentX - 100}%)`;
            });

            setTimeout(() => {
                imgs.forEach(img => {
                    const currentX =
                        parseInt(img.style.transform.replace('translateX(', '')) || 0;

                    if (currentX < 0) {
                        img.style.transition = 'none';
                        img.style.transform = 'translateX(200%)';
                    }
                });

                isAnimating = false;
            }, 550);
        });
    });
}

startSlider();