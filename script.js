document.getElementById('dataForm').addEventListener('submit', (e) => {
    e.preventDefault();
  
    const numberOfData = document.getElementById('numberOfData').value;
  
    fetch('/retrieve-data', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/x-www-form-urlencoded',
      },
      body: `numberOfData=${numberOfData}`,
    })
      .then((response) => response.json())
      .then((data) => {
        const resultDiv = document.getElementById('result');
        resultDiv.innerHTML = '';
  
        for (let i = 0; i < data.length; i++) {
          const p = document.createElement('p');
          p.textContent = JSON.stringify(data[i]);
          resultDiv.appendChild(p);
        }
      })
      .catch((error) => console.error(error));
  });
  