const tasksListElement = document.querySelector('.tasks__list');
const taskElements = tasksListElement
    ? Array.from(tasksListElement.querySelectorAll('.tasks__item'))
    : [];

let taskPositions = null;
let activeElement = null;
let shiftX = 0;
let shiftY = 0;

let positionsLoaded = false;

// --------------------------------------------------
// ИНИЦИАЛИЗАЦИЯ БЛОКОВ
// --------------------------------------------------

function initializeTasks(positions = {}) {
    if (!tasksListElement) {
        console.error('Не найден элемент .tasks__list');
        return;
    }

    taskElements.forEach((task, index) => {
        // Стабильный ID нужен для сохранения позиции в SQLite.
        if (!task.id) {
            task.id = `task-${index}`;
        }

        // Отключаем встроенное HTML-перетаскивание.
        task.draggable = false;

        // Устанавливаем начальные координаты.
        if (!task.style.left) {
            task.style.left = `${20 + index * 30}px`;
        }

        if (!task.style.top) {
            task.style.top = `${20 + index * 30}px`;
        }

        // Восстанавливаем сохранённую позицию.
        const saved = positions[task.id];

        if (saved) {
            task.style.left = `${saved.left}px`;
            task.style.top = `${saved.top}px`;
        }

        // Не назначаем один обработчик несколько раз.
        if (task.dataset.dragInitialized !== 'true') {
            task.addEventListener('mousedown', startDrag);
            task.dataset.dragInitialized = 'true';
        }
    });

    console.log('Блоки готовы к перетаскиванию:', taskElements.length);
}

// Инициализируем блоки сразу, не ожидая подключения к C++.
initializeTasks({});

// --------------------------------------------------
// ПОДКЛЮЧЕНИЕ К QT WEBCHANNEL
// --------------------------------------------------

function initDragTasks(channel) {
    if (!channel || !channel.objects) {
        console.error('Некорректный объект Qt WebChannel');
        return;
    }

    taskPositions = channel.objects.taskPositions;

    if (!taskPositions) {
        console.error('taskPositions не зарегистрирован в C++');
        return;
    }

    if (positionsLoaded) {
        return;
    }

    positionsLoaded = true;

    taskPositions.loadPositions(function (positions) {
        positions = positions || {};

        taskElements.forEach((task, index) => {
            const saved = positions[task.id];

            if (saved) {
                task.style.left = `${saved.left}px`;
                task.style.top = `${saved.top}px`;
            } else {
                // Сохраняем начальную позицию только после загрузки
                // существующих позиций из базы.
                taskPositions.savePosition(
                    task.id,
                    parseFloat(task.style.left) || 0,
                    parseFloat(task.style.top) || 0
                );
            }
        });

        console.log('Позиции блоков загружены из SQLite');
    });
}

// Используем только общий канал, созданный в bridge.js.
if (window.appChannelReady) {
    window.appChannelReady
        .then(initDragTasks)
        .catch(error => {
            console.error('Ошибка подключения WebChannel:', error);
        });
} else {
    console.warn(
        'appChannelReady не найден. Перетаскивание работает без сохранения позиций.'
    );
}

// --------------------------------------------------
// НАЧАЛО ПЕРЕТАСКИВАНИЯ
// --------------------------------------------------

function startDrag(evt) {
    // Только левая кнопка мыши.
    if (evt.button !== 0) {
        return;
    }

    // Не перетаскиваем блок при взаимодействии с чатом.
    if (evt.target.closest(
        '.chat-input, .send-btn, .chat-messages'
    )) {
        return;
    }

    activeElement = evt.currentTarget;

    activeElement.classList.add('selected');

    const rect = activeElement.getBoundingClientRect();

    shiftX = evt.clientX - rect.left;
    shiftY = evt.clientY - rect.top;

    document.addEventListener('mousemove', onDrag);
    document.addEventListener('mouseup', stopDrag);

    evt.preventDefault();
}

// --------------------------------------------------
// ПЕРЕМЕЩЕНИЕ БЛОКА
// --------------------------------------------------

function onDrag(evt) {
    if (!activeElement || !tasksListElement) {
        return;
    }

    const parentRect = tasksListElement.getBoundingClientRect();

    let newLeft = evt.clientX - parentRect.left - shiftX;
    let newTop = evt.clientY - parentRect.top - shiftY;

    const maxLeft = Math.max(
        0,
        tasksListElement.clientWidth - activeElement.offsetWidth
    );

    const maxTop = Math.max(
        0,
        tasksListElement.clientHeight - activeElement.offsetHeight
    );

    newLeft = Math.max(0, Math.min(newLeft, maxLeft));
    newTop = Math.max(0, Math.min(newTop, maxTop));

    activeElement.style.left = `${newLeft}px`;
    activeElement.style.top = `${newTop}px`;
}

// --------------------------------------------------
// ЗАВЕРШЕНИЕ ПЕРЕТАСКИВАНИЯ И СОХРАНЕНИЕ ПОЗИЦИИ
// --------------------------------------------------

function stopDrag() {
    if (activeElement) {
        const task = activeElement;

        task.classList.remove('selected');

        if (
            taskPositions &&
            typeof taskPositions.savePosition === 'function'
        ) {
            taskPositions.savePosition(
                task.id,
                parseFloat(task.style.left) || 0,
                parseFloat(task.style.top) || 0
            );
        }

        activeElement = null;
    }

    document.removeEventListener('mousemove', onDrag);
    document.removeEventListener('mouseup', stopDrag);
}

// --------------------------------------------------
// ЗАПРЕТ СТАНДАРТНОГО ПЕРЕТАСКИВАНИЯ ИЗОБРАЖЕНИЙ
// --------------------------------------------------

document.querySelectorAll('img').forEach(img => {
    img.addEventListener('dragstart', evt => {
        evt.preventDefault();
    });
});

// --------------------------------------------------
// ЧАСЫ
// --------------------------------------------------

function updateClock() {
    const clock = document.getElementById('clock');

    if (!clock) {
        return;
    }

    const now = new Date();

    clock.textContent = now.toLocaleTimeString('ru-RU', {
        hour: '2-digit',
        minute: '2-digit',
        second: '2-digit'
    });

    const seconds = now.getSeconds();
    const minutes = now.getMinutes();
    const hours = now.getHours() % 12;

    const secondHand = document.getElementById('second-hand');
    const minuteHand = document.getElementById('minute-hand');
    const hourHand = document.getElementById('hour-hand');

    if (secondHand) {
        secondHand.style.transform = `rotate(${seconds * 6}deg)`;
    }

    if (minuteHand) {
        minuteHand.style.transform =
            `rotate(${minutes * 6 + seconds * 0.1}deg)`;
    }

    if (hourHand) {
        hourHand.style.transform =
            `rotate(${hours * 30 + minutes * 0.5}deg)`;
    }
}

updateClock();
setInterval(updateClock, 1000);
