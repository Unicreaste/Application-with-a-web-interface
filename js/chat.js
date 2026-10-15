(function () {
            const input    = document.getElementById('chat-input');
            const sendBtn  = document.getElementById('send-btn');
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