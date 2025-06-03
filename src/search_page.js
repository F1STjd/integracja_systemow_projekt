window.addEventListener('load', function() {
  const overlay = document.getElementById('loadingOverlay');
  if (overlay) {
    overlay.classList.remove('active');
  }
});

document.querySelectorAll('.price-button').forEach(button => {
  button.addEventListener('click', function(e) {
    const overlay = document.getElementById('loadingOverlay');
    if (overlay) {
      overlay.classList.add('active');
    }
  });
});
