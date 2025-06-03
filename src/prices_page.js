let table = new DataTable('#result-table', {
  // config options...
});

const pageOverlay = document.getElementById('pageLoadingOverlay');

setTimeout(() => {
  if (pageOverlay) {
    pageOverlay.classList.remove('active');
  }
}, 2000);
