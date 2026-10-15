(function () {
    if (typeof qt === 'undefined' || !qt.webChannelTransport) {
        console.error('Qt WebChannel transport недоступен');
        return;
    }

    if (typeof QWebChannel === 'undefined') {
        console.error('Библиотека QWebChannel не загружена');
        return;
    }

    // Promise создаётся СРАЗУ, до ответа от Qt.
    window.appChannelReady = new Promise((resolve, reject) => {
        try {
            new QWebChannel(qt.webChannelTransport, function (channel) {
                window.appChannel = channel;

                console.log('WebChannel подключён');
                console.log(
                    'Доступные объекты:',
                    Object.keys(channel.objects)
                );

                resolve(channel);
            });
        } catch (error) {
            reject(error);
        }
    });
})();