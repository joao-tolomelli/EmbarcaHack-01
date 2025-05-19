// src/routes/medicamento.routes.js
import express from 'express';
import jwtMiddleware from '../middleware/jwt.middleware.js';
import { permitOnly } from '../middleware/role.middleware.js';
import { validate } from '../middleware/validate.middleware.js';
import {
  medicamentoSchema,
  statusSchema
} from '../validators/medicamento.validator.js';

import {
  getTodosMedicamentos,
  postMedicamento,
  putMedicamento,
  deleteMedicamento,
  getProximosMedicamentos,
  putStatusMedicamento
} from '../controllers/medicamento.controller.js';

const router = express.Router();

//router.use(jwtMiddleware);

// GET /medicamentos (aberto a todos os usuários autenticados)
router.get('/', getTodosMedicamentos);

// POST /medicamentos (restrito)
router.post(
  '/',
//  permitOnly(0, 1, 2),
  validate(medicamentoSchema),
  postMedicamento
);

// PUT /medicamentos/:id (restrito)
router.put(
  '/:id',
//  permitOnly(0, 1, 2),
  validate(medicamentoSchema),
  putMedicamento
);

// DELETE /medicamentos/:id (restrito)
router.delete(
  '/:id',
//  permitOnly(0, 1, 2),
  deleteMedicamento
);

// PUT /medicamentos/status/:id (todos os usuários podem atualizar o status de seu próprio medicamento)
router.put(
  '/status/:id',
  validate(statusSchema),
  putStatusMedicamento
);

// GET /medicamentos/proximos/:pacienteId (sem restrição)
router.get('/proximos/:pacienteId', getProximosMedicamentos);

export default router;
