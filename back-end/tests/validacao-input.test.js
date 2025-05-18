import request from 'supertest';
import express from 'express';
import authRoutes from '../src/routes/auth.routes.js';
import medicamentoRoutes from '../src/routes/medicamento.routes.js';
import pool from '../src/config/db.js';

const app = express();
app.use(express.json());
app.use('/api/v1/auth', authRoutes);
app.use('/api/v1/medicamentos', medicamentoRoutes);

let token = '';

beforeAll(async () => {
   // Cria usuário de teste
   await pool.query('DELETE FROM users WHERE email = ?', ['validador@teste.com']);
   await request(app).post('/api/v1/auth/register').send({
      name: 'Validador',
      email: 'validador@teste.com',
      password: 'senha123',
      profile: 1
   });

   const res = await request(app).post('/api/v1/auth/login').send({
      email: 'validador@teste.com',
      password: 'senha123'
   });

   token = res.body.token;
});

afterAll(async () => {
   await pool.query('DELETE FROM medicamentos WHERE paciente_id IN (SELECT id FROM users WHERE email = ?)', ['validador@teste.com']);
   await pool.query('DELETE FROM users WHERE email = ?', ['validador@teste.com']);
   await pool.end();
});

describe('Validação de entrada com Joi - Medicamentos', () => {
   test('Rejeita medicamento sem campo obrigatório', async () => {
      const res = await request(app)
         .post('/api/v1/medicamentos')
         .set('Authorization', `Bearer ${token}`)
         .send({ name: 'Teste sem cor', time: '08:00' });

      expect(res.statusCode).toBe(400);
      expect(res.body.message).toMatch(/inválidos/i);
   });

   test('Rejeita horário mal formatado', async () => {
      const res = await request(app)
         .post('/api/v1/medicamentos')
         .set('Authorization', `Bearer ${token}`)
         .send({ name: 'Teste', color: 'azul', time: '99:99' });

      expect(res.statusCode).toBe(400);
      expect(res.body.errors).toEqual(expect.arrayContaining([
         expect.stringMatching(/formato HH:MM/)
      ]));
   });

   test('Rejeita status inválido', async () => {
      // Cria medicamento válido primeiro
      const med = await request(app)
         .post('/api/v1/medicamentos')
         .set('Authorization', `Bearer ${token}`)
         .send({ name: 'Validação', color: 'verde', time: '07:00' });

      const id = med.body.id;

      const res = await request(app)
         .put(`/api/v1/medicamentos/status/${id}`)
         .set('Authorization', `Bearer ${token}`)
         .send({ status: 'tomado' }); // inválido

      expect(res.statusCode).toBe(400);
      expect(res.body.message).toMatch(/inválido/i);
   });
});

describe('Validação de entrada com Joi - Autenticação', () => {
   test('Rejeita e-mail mal formatado no registro', async () => {
      const res = await request(app).post('/api/v1/auth/register').send({
         name: 'Teste',
         email: 'invalidemail',
         password: 'senha123',
         profile: 0
      });

      expect(res.statusCode).toBe(400);
      expect(res.body.errors).toEqual(expect.arrayContaining([
         expect.stringMatching(/email/i)
      ]));
   });

   test('Rejeita login com senha faltando', async () => {
      const res = await request(app).post('/api/v1/auth/login').send({
         email: 'validador@teste.com'
      });

      expect(res.statusCode).toBe(400);
      expect(res.body.errors).toEqual(expect.arrayContaining([
         expect.stringMatching(/(senha|password)/i)
      ]));
   });
});
