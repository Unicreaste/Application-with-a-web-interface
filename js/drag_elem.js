const tasksListElement = document.querySelector('.tasks__list');
const taskElements = tasksListElement
    ? Array.from(tasksListElement.querySelectorAll('.tasks__item'))
    : [];

let taskPositions = null;
let activeElement = null;
let shiftX = 0;
let shiftY = 0;

let positionsLoaded = false;

let positionsReady = false;
let emptyBlockCounter = 0;

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

// Создание пустого блока
function addEmptyBlock(id = null) {
    if (!tasksListElement) return null;

    const block = document.createElement('li');

    block.className = 'tasks__item empty-block';

    block.id = id || `task-empty-${Date.now()}-${++emptyBlockCounter}`;

    block.setAttribute('aria-label', 'Пустой блок');

    const index = taskElements.length;
    const offset = 20 + index * 30;

    const maxLeft = Math.max(0, tasksListElement.clientWidth - 200);
    const maxTop = Math.max(0, tasksListElement.clientHeight - 200);

    block.style.left = `${Math.min(offset, maxLeft)}px`;
    block.style.top = `${Math.min(offset, maxTop)}px`;

    block.draggable = false;

    // Подключаем существующее перетаскивание
    block.addEventListener('mousedown', startDrag);
    block.dataset.dragInitialized = 'true';

    tasksListElement.appendChild(block);

    // Добавляем в общий массив элементов
    taskElements.push(block);

    return block;
}


// Обработчик кнопки "+"
function createEmptyBlock() {
    const block = addEmptyBlock();

    if (!block) return;

    // Сохраняем координаты нового блока
    if (
        taskPositions &&
        positionsReady &&
        typeof taskPositions.savePosition === 'function'
    ) {
        taskPositions.savePosition(
            block.id,
            parseFloat(block.style.left) || 0,
            parseFloat(block.style.top) || 0
        );
    }
}


// Подключаем кнопку
const addBlockButton = document.getElementById('add-block-btn');

if (addBlockButton) {
    addBlockButton.addEventListener('click', createEmptyBlock);
}

// ==========================================
// КОНТЕКСТНОЕ МЕНЮ БЛОКОВ
// ==========================================

let blockContextMenu = null;


// Закрытие меню
function closeBlockContextMenu() {
    if (blockContextMenu) {
        blockContextMenu.remove();
        blockContextMenu = null;
    }
}


// Удаление блока
function deleteTaskBlock(block) {
    const blockId = block.id;

    // Удаляем запись из SQLite
    if (
        taskPositions &&
        typeof taskPositions.deletePosition === 'function'
    ) {
        taskPositions.deletePosition(blockId);
    }

    // Удаляем элемент с рабочего поля
    block.remove();

    // Удаляем из общего массива элементов
    const index = taskElements.indexOf(block);

    if (index !== -1) {
        taskElements.splice(index, 1);
    }
}


// Открытие меню правой кнопкой мыши
function showBlockContextMenu(event) {

    const block = event.target.closest('.tasks__item');

    if (!block || !tasksListElement.contains(block)) {
        return;
    }

    // Не мешаем работе полей ввода и элементов управления
    if (event.target.closest(
        'input, textarea, [contenteditable="true"], button, select, audio, a, .chat-messages'
    )) {
        return;
    }

    event.preventDefault();

    closeBlockContextMenu();

    // Создаём меню
    const menu = document.createElement('div');

    menu.className = 'block-context-menu';

    // Кнопка удаления
    const deleteButton = document.createElement('button');

    deleteButton.type = 'button';
    deleteButton.className = 'block-context-menu__delete';
    deleteButton.textContent = 'Удалить';

    deleteButton.addEventListener('click', () => {

        deleteTaskBlock(block);

        closeBlockContextMenu();
    });

    menu.appendChild(deleteButton);

    document.body.appendChild(menu);

    // Позиционируем рядом с курсором
    const left = Math.max(
        8,
        Math.min(
            event.clientX,
            window.innerWidth - menu.offsetWidth - 8
        )
    );

    const top = Math.max(
        8,
        Math.min(
            event.clientY,
            window.innerHeight - menu.offsetHeight - 8
        )
    );

    menu.style.left = `${left}px`;
    menu.style.top = `${top}px`;

    blockContextMenu = menu;
}


// Слушаем ПКМ
document.addEventListener(
    'contextmenu',
    showBlockContextMenu
);


// Закрываем меню при нажатии за его пределами
document.addEventListener('mousedown', event => {

    if (
        blockContextMenu &&
        !blockContextMenu.contains(event.target)
    ) {
        closeBlockContextMenu();
    }

});


// Закрываем по Escape
document.addEventListener('keydown', event => {

    if (event.key === 'Escape') {
        closeBlockContextMenu();
    }

});


// Закрываем при прокрутке или изменении размера окна
document.addEventListener(
    'scroll',
    closeBlockContextMenu,
    true
);

window.addEventListener(
    'resize',
    closeBlockContextMenu
);

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

        // Получаем список удалённых элементов
        const deletedTaskIds = new Set(
            positions.__deletedTasks || []
        );

        // Убираем удалённые стандартные блоки из HTML
        [...taskElements].forEach(task => {

            if (deletedTaskIds.has(task.id)) {

                task.remove();

                const index = taskElements.indexOf(task);

                if (index !== -1) {
                    taskElements.splice(index, 1);
                }
            }

        });

        // Восстанавливаем ранее созданные пустые блоки
        Object.keys(positions).forEach(taskId => {
            if (
                taskId.startsWith('task-empty-') &&
                !document.getElementById(taskId)
            ) {
                addEmptyBlock(taskId);
            }
        });

        taskElements.forEach((task, index) => {

            const saved = positions[task.id];

            if (saved) {
                task.style.left = `${saved.left}px`;
                task.style.top = `${saved.top}px`;
            } else {
                taskPositions.savePosition(
                    task.id,
                    parseFloat(task.style.left) || 0,
                    parseFloat(task.style.top) || 0
                );
            }

        });

        positionsReady = true;

        console.log('Позиции блоков загружены из SQLite');

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

        positionsReady = true;
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

    // Только левая кнопка мыши
    if (evt.button !== 0) {
        return;
    }

    // Не начинаем перетаскивание при взаимодействии
    // с интерактивными элементами
    if (evt.target.closest(
        'input, button, select, textarea, audio, a, label'
    )) {
        return;
    }

    // Не перетаскиваем блок при взаимодействии с чатом
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

    const now = new Date();

    // Цифровое время
    const clock = document.getElementById('clock');

    if (clock) {
        clock.textContent = now.toLocaleTimeString('ru-RU', {
            hour: '2-digit',
            minute: '2-digit',
            second: '2-digit'
        });
    }

    // День недели
    const weekday = document.getElementById('clock-weekday');

    if (weekday) {
        weekday.textContent = now.toLocaleDateString('ru-RU', {
            weekday: 'long'
        });
    }

    // Дата
    const date = document.getElementById('clock-date');

    if (date) {
        date.textContent = now.toLocaleDateString('ru-RU', {
            day: 'numeric',
            month: 'long',
            year: 'numeric'
        });
    }

    // Аналоговые стрелки
    const seconds =
        now.getSeconds() + now.getMilliseconds() / 1000;

    const minutes = now.getMinutes();
    const hours = now.getHours() % 12;

    const secondHand = document.getElementById('second-hand');
    const minuteHand = document.getElementById('minute-hand');
    const hourHand = document.getElementById('hour-hand');

    if (secondHand) {
        secondHand.style.transform =
            `rotate(${seconds * 6}deg)`;
    }

    if (minuteHand) {
        minuteHand.style.transform =
            `rotate(${minutes * 6 + seconds * 0.1}deg)`;
    }

    if (hourHand) {
        hourHand.style.transform =
            `rotate(${hours * 30 + minutes * 0.5 + seconds / 120}deg)`;
    }
}

function createClockMarks() {

    const container = document.getElementById('clock-marks');

    if (!container || container.children.length > 0) {
        return;
    }

    for (let i = 0; i < 60; i++) {

        const mark = document.createElement('span');

        mark.className = i % 5 === 0
            ? 'clock-mark major'
            : 'clock-mark';

        mark.style.transform = `rotate(${i * 6}deg)`;

        container.appendChild(mark);
    }
}

createClockMarks();

updateClock();
setInterval(updateClock, 50);
