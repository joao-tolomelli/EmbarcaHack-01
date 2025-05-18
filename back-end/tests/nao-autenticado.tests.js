import request from 'supertest';
import express from 'express';
import apiRoutes from '../src/routes/api.routes.js';
import medicamentoRoutes from '../src/routes/medicamento.routes.js';
import authRoutes from '../src/routes/auth.routes.js';

const app = express();
app.use(express.json());
app.use('/api/v1', apiRoutes); // inclui /status e /private
app.use('/api/v1/medicamentos', medicamentoRoutes);
app.use('/api/v1/auth', authRoutes);

describe('Acesso - Usuário não autenticado', () => {
  test('Pode acessar /api/v1/status (público)', async () => {
    const res = await request(app).get('/api/v1/status');
    expect(res.statusCode).toBe(200);
    expect(res.body.status).toMatch(/online/i);
  });

  test('Não pode acessar /api/v1/private (protegido)', async () => {
    const res = await request(app).get('/api/v1/private');
    expect(res.statusCode).toBe(401);
    expect(res.body.message).toMatch(/token/i);
  });

  test('Não pode acessar /api/v1/medicamentos', async () => {
    const res = await request(app).get('/api/v1/medicamentos');
    expect(res.statusCode).toBe(401);
  });

  test('Não pode acessar POST /api/v1/medicamentos', async () => {
    const res = await request(app)
      .post('/api/v1/medicamentos')
      .send({ name: 'Teste', color: 'verde', time: '08:00' });
    expect(res.statusCode).toBe(401);
  });

  test('Não pode acessar PUT /api/v1/medicamentos/status/:id', async () => {
    const res = await request(app)
      .put('/api/v1/medicamentos/status/1')
      .send({ status: 'ingerido' });
    expect(res.statusCode).toBe(401);
  });
});
