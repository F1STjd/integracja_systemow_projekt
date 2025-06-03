function generateColors(count) {
  const colors = [];
  const borderColors = [];
  for (let i = 0; i < count; i++) {
    const hue = (i * 360 / count) % 360;
    const saturation = 65 + (i % 3) * 10;
    const lightness = 50 + (i % 2) * 10;
    colors.push(`hsla(${hue}, ${saturation}%, ${lightness}%, 0.7)`);
    borderColors.push(`hsla(${hue}, ${saturation}%, ${lightness - 10}%, 1)`);
  }
  return { backgroundColor: colors, borderColor: borderColors };
}

function createAffordabilityChart(chartLabels, chartData, countryNames) {
  const ctx = document.getElementById('affordabilityChart');
  const chartColors = generateColors(chartLabels.length);

  new Chart(ctx, {
    type: 'bar',
    data: {
      labels: chartLabels,
      datasets: [{
        label: 'Copies per Year',
        data: chartData,
        backgroundColor: chartColors.backgroundColor,
        borderColor: chartColors.borderColor,
        borderWidth: 1
      }]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      scales: {
        y: {
          beginAtZero: true,
          title: {
            display: true,
            text: 'Number of Copies'
          }
        },
        x: {
          title: {
            display: true,
            text: 'Country Code'
          }
        }
      },
      plugins: {
        title: {
          display: true,
          text: 'Game Affordability by Country'
        },
        legend: {
          display: false
        },
        tooltip: {
          callbacks: {
            title: function(context) {
              const index = context[0].dataIndex;
              return countryNames[index] + ' (' + chartLabels[index] + ')';
            },
            label: function(context) {
              return 'Copies per Year: ' + context.parsed.y.toFixed(1);
            }
          }
        }
      }
    }
  });
}
