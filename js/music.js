const audio = document.getElementById('audio');

const playBtn = document.getElementById('play');
const prevBtn = document.getElementById('prev');
const nextBtn = document.getElementById('next');

const progress = document.getElementById('progress');
const volume = document.getElementById('volume');

const currentTimeEl = document.getElementById('currentTime');
const durationEl = document.getElementById('duration');

const trackName = document.getElementById('trackName');
const trackArtist = document.getElementById('trackArtist');


const playIcon = `<svg viewBox="0 0 24 24">         <path d="M8 5L20 12L8 19V5Z" />     </svg>`;

const pauseIcon = `<svg viewBox="0 0 24 24">         <path d="M7 5H10V19H7Z" />         <path d="M14 5H17V19H14Z" />     </svg>`;

const trackCover = document.getElementById('trackCover');
const coverFallback = document.getElementById('coverFallback');

let currentCoverUrl = null;
let metadataRequestId = 0;

function loadTrackMetadata(track) {
    const requestId = ++metadataRequestId;
    trackCover.classList.remove('loaded');
    coverFallback.style.display = 'block';
    trackCover.removeAttribute('src');
    if (currentCoverUrl) {
        URL.revokeObjectURL(currentCoverUrl);
        currentCoverUrl = null;
    }

    trackName.textContent = 'Loading...';
    trackArtist.textContent = 'Loading...';
    if (!window.jsmediatags) {
        console.error('jsmediatags не подключена');
        trackName.textContent = 'Unknown track';
        trackArtist.textContent = 'Unknown artist';
        return;
    }
    window.appChannelReady.then(channel => {

        if (requestId !== metadataRequestId) return;

        // Получаем C++-объект
        const musicBridge = channel.objects.musicBridge;

        if (!musicBridge) {
            throw new Error('MusicBridge не зарегистрирован в C++');
        }

        console.log('Запрашиваем файл:', track.src);

        // Просим C++ прочитать файл
        musicBridge.readFileBase64(track.src, function (base64) {

            if (requestId !== metadataRequestId) return;

            if (!base64) {
                console.error('C++ не смог прочитать музыкальный файл');
                trackName.textContent = 'Unknown track';
                trackArtist.textContent = 'Unknown artist';
                return;
            }

            // Превращаем Base64 в бинарные данные
            const binary = atob(base64);
            const bytes = new Uint8Array(binary.length);

            for (let i = 0; i < binary.length; i++) {
                bytes[i] = binary.charCodeAt(i);
            }

            // Создаём Blob из содержимого файла
            const blob = new Blob(
                [bytes],
                { type: 'audio/mp4' }
            );

            console.log('Файл получен от C++:', blob.size, 'байт');

            // Читаем метаданные
            window.jsmediatags.read(blob, {

                onSuccess: function (result) {
                    if (requestId !== metadataRequestId) return;

                    const tags = result.tags;

                    trackName.textContent =
                        tags.title || 'Unknown track';

                    trackArtist.textContent =
                        tags.artist || 'Unknown artist';

                    // Получаем обложку
                    const picture = tags.picture;

                    if (!picture || !picture.data) {
                        console.log('У композиции нет встроенной обложки');
                        return;
                    }

                    const imageBlob = new Blob(
                        [new Uint8Array(picture.data)],
                        { type: picture.format }
                    );

                    currentCoverUrl = URL.createObjectURL(imageBlob);

                    trackCover.onload = function () {
                        trackCover.classList.add('loaded');
                        coverFallback.style.display = 'none';
                    };

                    trackCover.onerror = function () {
                        console.error('Не удалось отобразить обложку');
                        trackCover.classList.remove('loaded');
                        coverFallback.style.display = 'block';
                    };

                    trackCover.src = currentCoverUrl;
                },

                onError: function (error) {
                    if (requestId !== metadataRequestId) return;

                    console.warn(
                        'Ошибка чтения метаданных:',
                        error.type,
                        error.info
                    );

                    trackName.textContent = 'Unknown track';
                    trackArtist.textContent = 'Unknown artist';
                }
            });
        });

    }).catch(error => {
        console.error('Ошибка MusicBridge:', error);

        if (requestId !== metadataRequestId) return;

        trackName.textContent = 'Unknown track';
        trackArtist.textContent = 'Unknown artist';
    });
}

const playlist = [
    {
        src: 'music/angel.mp3'
    },
    {
        src: 'music/chaos.exe.mp3'
    },
    {
        src: 'music/Devil Trigger.mp3'
    }
];

let currentTrack = 0;


function formatTime(time) {
    if (!Number.isFinite(time)) return '0:00';

    const minutes = Math.floor(time / 60);
    const seconds = Math.floor(time % 60);

    return `${minutes}:${String(seconds).padStart(2, '0')}`;
}

function loadTrack(index) {
    currentTrack = index;

    const track = playlist[currentTrack];

    audio.src = track.src;

    loadTrackMetadata(track);

    progress.value = 0;
    currentTimeEl.textContent = '0:00';
    durationEl.textContent = '0:00';

    playBtn.innerHTML = playIcon;

}


playBtn.addEventListener('click', async () => {
    if (audio.paused) {
        try {
            await audio.play();
        } catch (error) {
            console.error('Playback error:', error);

        }
    } else {
        audio.pause();
    }
});

audio.addEventListener('play', () => {
    playBtn.innerHTML = pauseIcon;
});

audio.addEventListener('pause', () => {
    playBtn.innerHTML = playIcon;

    if (!audio.ended) {
    }
});

audio.addEventListener('loadedmetadata', () => {
    durationEl.textContent = formatTime(audio.duration);
});

let isSeeking = false;
let wasPlaying = false;

progress.addEventListener('pointerdown', () => {
    isSeeking = true;

    wasPlaying = !audio.paused;
});

progress.addEventListener('input', () => {

    if (!isSeeking) return;

    if (Number.isFinite(audio.duration)) {
        const previewTime =
            (Number(progress.value) / 100) * audio.duration;

        currentTimeEl.textContent = formatTime(previewTime);
    }
});


window.addEventListener('pointerup', async () => {

    if (!isSeeking) return;

    isSeeking = false;

    if (!Number.isFinite(audio.duration)) return;


    const selectedTime =
        (Number(progress.value) / 100) * audio.duration;


    audio.pause();

    audio.currentTime = selectedTime;


    currentTimeEl.textContent = formatTime(selectedTime);

    if (wasPlaying) {
        try {
            await audio.play();
        } catch (error) {
            console.error('Playback error:', error);
        }
    }
});

audio.addEventListener('timeupdate', () => {

    if (!Number.isFinite(audio.duration)) return;

    // Пока пользователь выбирает время,
    // не трогаем положение ползунка
    if (isSeeking) return;

    progress.value =
        (audio.currentTime / audio.duration) * 100;

    currentTimeEl.textContent =
        formatTime(audio.currentTime);
});

volume.addEventListener('input', () => {
    audio.volume = Number(volume.value);
});

audio.volume = 0.7;

nextBtn.addEventListener('click', () => {
    loadTrack((currentTrack + 1) % playlist.length);
    audio.play().catch(console.error);
});

prevBtn.addEventListener('click', () => {
    if (audio.currentTime > 3) {
        audio.currentTime = 0;

        progress.value = 0;
        currentTimeEl.textContent = '0:00';

        return;
    }

    loadTrack(
        (currentTrack - 1 + playlist.length) % playlist.length
    );
    audio.play().catch(console.error);
});


audio.addEventListener('ended', () => {
    loadTrack((currentTrack + 1) % playlist.length);
    audio.play().catch(console.error);
});


loadTrack(0);