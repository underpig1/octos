hljs.highlightAll();

// umami
document.addEventListener('DOMContentLoaded', () => {
    const script = document.createElement('script');
    script.src = 'https://cloud.umami.is/script.js';
    script.defer = true;
    script.setAttribute('data-website-id', '416a23b5-11a1-49f5-9877-24803f746146');
    document.head.appendChild(script);

    document.querySelectorAll('button.md-feedback__icon').forEach(button => {
        button.addEventListener('click', () => {
            const value = button.getAttribute('data-md-value');
            umami.track(`feedback`, { url: window.location.pathname, value });
        });
    });
});