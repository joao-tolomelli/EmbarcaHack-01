const mqtt = require('mqtt');

// Conectando ao broker MQTT local — certifique-se da porta (geralmente é 1883, não 8080)
const client = mqtt.connect('mqtt://broker.hivemq.com:1883'); // Altere para a porta correta, se necessário

client.on('connect', () => {
  console.log('✅ Conectado ao broker MQTT');
});

client.on('error', (err) => {
  console.error('❌ Erro na conexão MQTT:', err.message);
});

// Função segura de publicação
function publish(topic, message) {
  if (client.connected) {
    client.publish(topic, message, (err) => {
      if (err) {
        console.error('❌ Erro ao publicar:', err.message);
      } else {
        console.log(`📤 Publicado no tópico '${topic}':`, message);
      }
    });
  } else {
    console.warn('⚠️ Cliente MQTT não está conectado. Não foi possível publicar.');
  }
}

module.exports = {
  client,
  publish,
};
