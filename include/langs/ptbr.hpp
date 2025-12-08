#include <array>

namespace PTBR {
	const std::array strings = {
		//Main Menu
		"Deletar configuração",
		"Deletar patches",
		"SaltyNX Não está funcionando!",
		"Não é possível detectar o plugin\nNX-FPS no cartão de memória!",
		"O Jogo não está rodando!",
		"Todos",
		"Tudo",
		"Lista de jogos",
		"Configurações de exibição",
		"Jogo foi fechado!\nSobreposição desativada!",
		"Jogo não está em execução!\nSobreposição desativada!",
		"Jogo está em execução.",
		"NX-FPS não está em execução!",
		"NX-FPS está em execução,\naguardando quadro.",
		"Reinicie a sobreposição\npara verificar novamente.",
		"NX-FPS está em execução.",
		"O patch agora força 60 Hz.",
		"Aumentar meta de FPS",
		"Diminuir meta de FPS",
		"Alterar meta de FPS",
		"Desativar meta de FPS personalizada",
		"Configurações avançadas",
		"Modo de intervalo:\n\uE019 0 (Não utilizado)",
		"Modo de intervalo:\n\uE019 %d  (%.1f FPS)",
		"Modo de intervalo:\n\uE019 %d (%d FPS)",
		"Modo de intervalo:\n\uE019 %d (Incorreto)",
		"Meta de FPS personalizada:\n\uE019 Desativada",
		"Meta de FPS personalizada:\n\uE019 %d",

		//Advanced Settings
		"Set Buffering",
		"Será aplicado ao próximo iniciar do jogo.",
		"Dobro",
		"Triplo",
		"Triplo (forçado)",
		"Quádruplo (forçado)",
		"Quádruplo",
		"Sincronização de janela",
		"Modo",
		"Habilitado",
		"Semi-Habilitado",
		"Desabilitado",
		"Ligado",
		"Desligado",
		"Semi",
		"Arquivo de config não encontrado\nTID: %016lX\nBID: %016lX",
		"Erro na config do jogo: 0x%X",
		"Arquivo de patch não existe.\nUse \"Converter config para patch file\"\npara criá-lo!",
		"Arquivo de patch não existe.",
		"Nova config baixada com sucesso.\nUse \"Converter config para patch file\"\npara aplicá-la!",
		"Arquivo de patch existe.",
		"Interface de API GPU: NVN",
		"Espera de sincronização de janela",
		"Interface de API GPU: EGL",
		"Interface de API GPU: Vulkan",
		"Patches do FPSLocker",
		"Config válida encontrado!",
		"Reinicie o jogo após a conversão!",
		"Converter config para patch file",
		"Patch criado com sucesso.\nReinicie o jogo e altere\na meta de FPS para aplicar!",
		"Erro ao criar patch: 0x%x",
		"Excluir patch",
		"Patch excluído com sucesso.",
		"Isso pode levar até %d segundos.",
		"Verificar/baixar config",
		"Verificando Warehouse por config...",
		"Diversos",
		"Pausar jogo sem foco",
		"Patch aplicado ao jogo.",
		"Master Write aplicado ao jogo.",
		"Plugin não aplicou o patch ao jogo.",
		"Set/Ativo/Buffers disponíveis: %d/%d/%d",
		"Buffers ativos: %d",
		"Tempo de conexão esgotado!",
		"Configuração não disponível! RC: 0x%x",
		"Config não disponível!\nVerificando Warehouse...",
		"Config não disponível!\nVerificando Warehouse...\nTempo esgotado!",
		"Config não disponível!\nVerificando Warehouse...\nErro de conexão!",
		"Nenhuma nova config disponível.",
		"Internet não disponível!",
		"Patch não necessário para este jogo!",
		"Jogo não está no Warehouse!",
		"Jogo listado no Warehouse,\nmas com uma versão diferente.\n%s não precisa de patch,\ntalvez a sua também não precise!",
		"Jogo listado no Warehouse,\nmas com uma versão diferente.\n%s recomenda patch,\nmas a configuração não está disponível!",
		"Jogo listado no Warehouse,\nmas com uma versão diferente.\n%s tem configuração\ndisponível!",
		"Jogo no Warehouse recomenda patch\npara esta versão,\nporém config não disponível!",
		"Erro de conexão! RC: 0x%x",
		
		//Display Settings
		"Testador de frameskip",

		"Como utilizar:\n"
		"1. Use uma câmera com ajuste\n"
		"manual de tempo de exposição\n"
		"e ISO.\n"
		"2. Defina tempo de exposição para\n"
		"1/10s ou mais, e ISO para não ficar\n"
		"nem muito claro nem muito escuro\n"
		"(geralmente por volta de 50 para\n"
		"1/10s).\n"
		"3. Pressione \uE0E0 para continuar.\n"
		"4. Fotografe a tela.\n"
		"5. Se todos os blocos, exceto\n"
		"o primeiro e o último,\n"
		"tiverem brilho desigual,\n"
		"sua tela não suporta nativamente\n"
		"a taxa de atualização atual\n"
		"e está rodando em outra taxa.\n\n"
		"Lembre que, mesmo que sua tela\n"
		"esteja pulando frames, ainda é\n"
		"muito melhor do que usar uma meta\n"
		"de FPS inferior que não combine\n"
		"com sua taxa de atualização, pois\n"
		"solução de hardware divide melhor\n"
		"os tempos de quadro.",
		

		"Pressione \uE0E1 para sair",
		"Renderização demorando demais!\nFeche o jogo, volte à tela inicial,\ntente novamente.",
		
		"Este menu testará todas as taxas\n"
		"de atualização abaixo de 60 Hz:\n"
		"40, 45, 50, 55. Pressione o botão\n"
		"solicitado para confirmar o funcionamento.\n"
		"Se nada for pressionado em 15 s,\n"
		"ele passará à próxima taxa.",

		"Para iniciar, pressione X.",
		"Assistente de underclock de display",
		"Não suportado em %dp!",
		"Feche o jogo primeiro!",
		"Pressione ZL para confirmar %d Hz.",
		"Pressione X para confirmar %d Hz.",
		"Pressione Y para confirmar %d Hz.",
		"Pressione ZR para confirmar %d Hz.",

		"Este menu testará taxas acima de 60 Hz\n"
		"até %d Hz.\n\n"
		"Pressione o botão solicitado para\n"
		"confirmar. Se nada for pressionado\n"
		"em 10 s, testará a próxima taxa.\n"
		"Isso pode levar até %d s.",

		"Assistente de overclock de display",
		"Configurações manuais %dp Docked",
		"Configurações adicionais Docked",
		"Permitir patches forçarem 60 Hz",
		"Usar menor taxa para metas de FPS não correspondentes",
		"n/d",
		"Máx. taxa de atualização disponível: %u Hz\nmyDP link rate: %s\nConfig ID: %08X",
		"Configurações de display Docked",

		"Taxas %dp permitidas",
		"Assistente de overclock %dp",
		"Configurações adicionais",

		"Você não está no modo docked.\n"
		"Volte, encaixe seu Switch no dock\n"
		"e retorne.",

		"Alterar taxa de atualização",
		"Aumentar taxa de atualização",
		"Diminuir taxa de atualização",
		"Combinar taxa de atualização com meta de FPS.",
		"Sincronização de Tela Portátil",
		"Configurações de Docked",
		"Modo Retro Remake",
		"Taxa de atualização da tela: %d Hz",

		"ESTA É UMA FUNÇÃO\nEXPERIMENTAL!\n\n"
		"Pode causar danos irreparáveis\n"
		"à sua tela.\n\n"
		"Ao pressionar Aceitar, você assume\n"
		"toda responsabilidade por\n"
		"qualquer ocorrência decorrente\n"
		"desta ferramenta.",

		"Aviso de configurações de tela",
		"Recusar",
		"Aceitar",
		"Forçar língua Inglesa",
		"Sincronização de Tela em Dock",
		"Apenas no modo portátil",
		"60 Hz no menu HOME"
	};

	const std::array teslaStrings = {
	   "Voltar",
	   "OK" 
	};

	static_assert(teslaStrings.size() == 2);
}