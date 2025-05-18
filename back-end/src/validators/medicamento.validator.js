// src/validators/medicamento.validator.js
import Joi from 'joi';

export const medicamentoSchema = Joi.object({
  name: Joi.string().min(2).max(100).required(),
  color: Joi.string().min(3).max(50).required(),
  time: Joi.string()
    .pattern(/^([01][0-9]|2[0-3]):[0-5][0-9]$/)
    .required()
    .messages({
      'string.pattern.base': 'O horário deve estar no formato HH:MM entre 00:00 e 23:59'
    })
});

export const statusSchema = Joi.object({
  status: Joi.string().valid('pendente', 'ingerido', 'ignorado').required()
});

