import {
  listarMedicamentos,
  criarMedicamento,
  atualizarMedicamento,
  deletarMedicamento,
  listarProximosMedicamentos,
  atualizarStatusMedicamento,
  buscarMedicamentoPorId
} from '../models/medicamento.model.js';

export async function getTodosMedicamentos(req, res) {
  try {
    const medicamentos = await listarMedicamentos();
    res.json(medicamentos);
  } catch (err) {
    console.error('Erro ao listar medicamentos:', err);
    res.status(500).json({ message: 'Erro interno no servidor' });
  }
}

export async function postMedicamento(req, res) {
  const { name, color, time } = req.body;
  const paciente_id = req.user.id;

  try {
    const novo = await criarMedicamento({ name, color, time, paciente_id });
    res.status(201).json(novo);
  } catch (err) {
    console.error('Erro ao criar medicamento:', err);
    res.status(500).json({ message: 'Erro ao criar medicamento' });
  }
}

export async function putMedicamento(req, res) {
  const id = parseInt(req.params.id);
  const { name, color, time } = req.body;

  try {
    const medicamento = await buscarMedicamentoPorId(id);

    if (!medicamento) {
      return res.status(404).json({ message: 'Medicamento não encontrado' });
    }

    if (medicamento.paciente_id !== req.user.id) {
      return res.status(403).json({ message: 'Acesso negado' });
    }

    await atualizarMedicamento(id, { name, color, time });
    res.status(204).send();
  } catch (err) {
    console.error('Erro ao atualizar medicamento:', err);
    res.status(500).json({ message: 'Erro ao atualizar medicamento' });
  }
}

export async function deleteMedicamento(req, res) {
  const id = parseInt(req.params.id);

  try {
    const medicamento = await buscarMedicamentoPorId(id);

    if (!medicamento) {
      return res.status(404).json({ message: 'Medicamento não encontrado' });
    }

    if (medicamento.paciente_id !== req.user.id) {
      return res.status(403).json({ message: 'Acesso negado' });
    }

    await deletarMedicamento(id);
    res.status(204).send();
  } catch (err) {
    console.error('Erro ao deletar medicamento:', err);
    res.status(500).json({ message: 'Erro ao deletar medicamento' });
  }
}

export async function putStatusMedicamento(req, res) {
  const id = parseInt(req.params.id);
  const { status } = req.body;

  if (!['pendente', 'ingerido', 'ignorado'].includes(status)) {
    return res.status(400).json({ message: 'Status inválido' });
  }

  try {
    const medicamento = await buscarMedicamentoPorId(id);

    if (!medicamento) {
      return res.status(404).json({ message: 'Medicamento não encontrado' });
    }

    if (medicamento.paciente_id !== req.user.id) {
      return res.status(403).json({ message: 'Acesso negado' });
    }

    await atualizarStatusMedicamento(id, status);
    res.status(204).send();
  } catch (err) {
    console.error('Erro ao atualizar status:', err);
    res.status(500).json({ message: 'Erro ao atualizar status do medicamento' });
  }
}

export async function getProximosMedicamentos(req, res) {
  const pacienteId = parseInt(req.params.pacienteId);

  try {
    const lista = await listarProximosMedicamentos(pacienteId);
    res.json(lista);
  } catch (err) {
    console.error('Erro ao buscar próximos medicamentos:', err);
    res.status(500).json({ message: 'Erro ao buscar próximos medicamentos' });
  }
}

export default {
  getTodosMedicamentos,
  postMedicamento,
  putMedicamento,
  deleteMedicamento,
  putStatusMedicamento,
  getProximosMedicamentos  
};
