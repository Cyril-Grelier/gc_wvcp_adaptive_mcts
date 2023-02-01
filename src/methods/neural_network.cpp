#include "neural_network.hpp"

torch::Tensor solution_to_tensor(const Solution &solution) {
    torch::Tensor one_hot_encoding =
        torch::zeros({solution.max_nb_colors, Graph::g->nb_vertices});
    for (int vertex = 0; vertex < Graph::g->nb_vertices; ++vertex) {
        one_hot_encoding.index_put_({solution.color(vertex), vertex}, 1);
    }

    return one_hot_encoding;
}

InvariantColorLinear::InvariantColorLinear(int64_t size_input,
                                           int64_t size_output,
                                           int64_t nb_colors_)
    : linear1(register_module("linear1", torch::nn::Linear(size_input, size_output))),
      linear2(register_module("linear2", torch::nn::Linear(size_input, size_output))),
      nb_colors(nb_colors_) {

    // set weights to zero
    // for (auto &p : named_parameters()) {
    //     torch::NoGradGuard no_grad;
    //     p.value().zero_();
    // }

    // set weights and bias to random values
    torch::NoGradGuard noGrad;
    torch::nn::init::xavier_normal_(linear1->weight);
    torch::nn::init::xavier_normal_(linear2->weight);
    torch::nn::init::constant_(linear1->bias, 0.01);
    torch::nn::init::constant_(linear2->bias, 0.01);
}

torch::Tensor InvariantColorLinear::forward(torch::Tensor input) {
    return (nb_colors * linear1(input) +
            linear2(torch::unsqueeze(torch::mean(input, 1), 1))) /
           (nb_colors + 1);
}

NeuralNetwork::NeuralNetwork(int64_t nb_colors,
                             int64_t nb_vertices,
                             int64_t nb_operators) {
    int64_t size_layer1 = Graph::g->nb_vertices;
    int64_t size_layer2 = Graph::g->nb_vertices + nb_operators / 2;

    layers = register_module(
        "layers",
        torch::nn::Sequential(
            InvariantColorLinear(nb_vertices, size_layer1, nb_colors),
            torch::nn::LeakyReLU(
                // https://pytorch.org/docs/stable/generated/torch.nn.LeakyReLU.html
                // https://pytorch.org/cppdocs/api/classtorch_1_1nn_1_1_leaky_re_l_u.html
                torch::nn::LeakyReLUOptions().negative_slope(0.2).inplace(false)),
            InvariantColorLinear(size_layer1, size_layer2, nb_colors),
            torch::nn::LeakyReLU(
                torch::nn::LeakyReLUOptions().negative_slope(0.2).inplace(false)),
            InvariantColorLinear(size_layer2, nb_operators, nb_colors)));
}
std::vector<int> NeuralNetwork::predict(torch::Tensor input) {
    auto tensor_predictions = forward(input);

    auto size_output = tensor_predictions.sizes()[0];

    std::vector<int> predictions(size_output);

    for (int i = 0; i < size_output; ++i) {
        predictions[i] = tensor_predictions[i].item<int>();
    }

    return predictions;
}

torch::Tensor NeuralNetwork::forward(torch::Tensor input) {
    return torch::mean(layers->forward(input), 1);
}

SolutionDataset::SolutionDataset(const std::vector<torch::Tensor> &inputs_,
                                 const std::vector<torch::Tensor> &utility_operator_)
    : _inputs(inputs_), _utility_operator(utility_operator_), _size(_inputs.size()) {
}

torch::data::Example<> SolutionDataset::get(size_t index) {
    // return {inputs[index].clone(), outputs[index].clone()};
    return {_inputs[index], _utility_operator[index]};
}

torch::optional<size_t> SolutionDataset::size() const {
    return _size;
}
