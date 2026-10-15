const tasksListElement = document.querySelector('.tasks__list');
const taskElements = tasksListElement.querySelectorAll('.tasks__item');

// Счётчики для первичной раскладки блоков "лесенкой"
let offsetIndex = 0;

taskElements.forEach((task) => {
    // начальные координаты (можно поменять под себя)
    task.style.left = (20 + offsetIndex * 30) + 'px';
    task.style.top = (20 + offsetIndex * 30) + 'px';
    offsetIndex++;

    task.draggable = false; // отключаем HTML5 drag — будем двигать мышью
    task.addEventListener('mousedown', startDrag);
});

let activeElement = null;
let shiftX = 0;
let shiftY = 0;

function startDrag(evt) {
    // не мешаем вводу в чат
    if (evt.target.closest('.chat-input, .send-btn, .chat-messages')) return;

    // только левая кнопка мыши
    if (evt.button !== 0) return;

    activeElement = evt.currentTarget;
    activeElement.classList.add('selected');

    const rect = activeElement.getBoundingClientRect();
    const parentRect = tasksListElement.getBoundingClientRect();

    // смещение курсора внутри блока
    shiftX = evt.clientX - rect.left;
    shiftY = evt.clientY - rect.top;

    // запоминаем, что координаты считаем относительно родителя
    activeElement.dataset.parentLeft = parentRect.left;
    activeElement.dataset.parentTop = parentRect.top;

    document.addEventListener('mousemove', onDrag);
    document.addEventListener('mouseup', stopDrag);
}

function onDrag(evt) {
    if (!activeElement) return;

    const parentRect = tasksListElement.getBoundingClientRect();

    let newLeft = evt.clientX - parentRect.left - shiftX;
    let newTop = evt.clientY - parentRect.top - shiftY;

    // ограничим в пределах списка (по желанию можно убрать)
    const maxLeft = tasksListElement.clientWidth - activeElement.offsetWidth;
    const maxTop = tasksListElement.clientHeight - activeElement.offsetHeight;

    newLeft = Math.max(0, Math.min(newLeft, maxLeft));
    newTop = Math.max(0, Math.min(newTop, maxTop));

    activeElement.style.left = newLeft + 'px';
    activeElement.style.top = newTop + 'px';
}

function stopDrag() {
    if (activeElement) {
        activeElement.classList.remove('selected');
        activeElement = null;
    }
    document.removeEventListener('mousemove', onDrag);
    document.removeEventListener('mouseup', stopDrag);
}

// запрещаем стандартное перетаскивание картинок
document.querySelectorAll('img').forEach(img => {
    img.addEventListener('dragstart', e => e.preventDefault());
});

// <--------------------- содержание блоков ----------------------->


// <--------------------- часы ----------------------->

function updateClock() {
    const now = new Date();

    // Цифровое время по часовому поясу устройства
    const timeString = now.toLocaleTimeString('ru-RU', {
        hour: '2-digit',
        minute: '2-digit',
        second: '2-digit'
    });

    document.getElementById('clock').textContent = timeString;

    // Положение стрелок
    const seconds = now.getSeconds();
    const minutes = now.getMinutes();
    const hours = now.getHours() % 12;

    const secondAngle = seconds * 6;
    const minuteAngle = minutes * 6 + seconds * 0.1;
    const hourAngle = hours * 30 + minutes * 0.5;

    document.getElementById('second-hand')
        .style.transform = `rotate(${secondAngle}deg)`;

    document.getElementById('minute-hand')
        .style.transform = `rotate(${minuteAngle}deg)`;

    document.getElementById('hour-hand')
        .style.transform = `rotate(${hourAngle}deg)`;
}

updateClock();
setInterval(updateClock, 1000);



// <--------------------- картинка ----------------------->


(function () {
    const input = document.getElementById('chat-input');
    const sendBtn = document.getElementById('send-btn');
    const messages = document.getElementById('chat-messages');

    function sendMessage() {
        const text = input.innerText.trim();
        if (!text) return;

        const msg = document.createElement('div');
        msg.className = 'message own';
        msg.textContent = text;
        messages.appendChild(msg);

        // очистить поле
        input.innerHTML = '';

        // прокрутить вниз
        messages.scrollTop = messages.scrollHeight;

        // вернуть фокус в поле ввода
        input.focus();
    }

    sendBtn.addEventListener('click', sendMessage);

    // Enter — отправить, Shift+Enter — перенос строки
    input.addEventListener('keydown', (e) => {
        if (e.key === 'Enter' && !e.shiftKey) {
            e.preventDefault();
            sendMessage();
        }
    });
})();