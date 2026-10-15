
(function () {
    const input = document.getElementById('chat-input');
    const sendBtn = document.getElementById('send-btn');
    const messages = document.getElementById('chat-messages');

    let chatManager = null;

    function addMessage(text, sender) {
        const msg = document.createElement('div');

        msg.className = sender === 'user'
            ? 'message own'
            : 'message';

        msg.textContent = text;
        messages.appendChild(msg);
    }

    function scrollToBottom() {
        messages.scrollTop = messages.scrollHeight;
    }

    function sendMessage() {
        const text = input.innerText.trim();

        if (!text) return;

        if (!chatManager) {
            console.error('ChatManager ещё не подключён');
            return;
        }

        addMessage(text, 'user');
        scrollToBottom();

        chatManager.saveMessage('user', text, function (success) {
            if (!success) {
                console.error('Не удалось сохранить сообщение');
            }
        });

        input.innerHTML = '';
        input.focus();
    }

    sendBtn.addEventListener('click', sendMessage);

    input.addEventListener('keydown', (e) => {
        if (e.key === 'Enter' && !e.shiftKey) {
            e.preventDefault();
            sendMessage();
        }
    });

    if (!window.appChannelReady) {
        console.error('appChannelReady не найден. Проверь js/bridge.js');
        return;
    }

    window.appChannelReady
        .then(channel => {
            // Важно: присваиваем внешней переменной, не создаём const.
            chatManager = channel.objects.chatManager;

            if (!chatManager) {
                console.error('chatManager не зарегистрирован в C++');
                return;
            }

            console.log('ChatManager подключён');

            chatManager.loadMessages(function (savedMessages) {
                messages.innerHTML = '';

                (savedMessages || []).forEach(function (item) {
                    addMessage(item.message, item.sender);
                });

                scrollToBottom();
            });
        })
        .catch(error => {
            console.error('Ошибка подключения ChatManager:', error);
        });
})();
