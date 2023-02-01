#pragma once

#include <torch/torch.h>

// import fmt here to avoid importing it everywhere
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include <fmt/printf.h>
#pragma GCC diagnostic pop

#include "../representation/Solution.hpp"
#include "../utils/utils.hpp"

struct InvariantColorLinear : torch::nn::Module {
    torch::nn::Linear linear1;
    torch::nn::Linear linear2;
    int64_t nb_colors;

    InvariantColorLinear(int64_t size_input, int64_t size_output, int64_t nb_colors_);

    torch::Tensor forward(torch::Tensor input);
};

struct NeuralNetwork : torch::nn::Module {
    torch::nn::Sequential layers;

    NeuralNetwork(int64_t nb_colors, int64_t nb_vertices, int64_t nb_operators);

    /**
     * @brief Use for helper
     *
     * @param input
     * @return std::vector<int>
     */
    std::vector<int> predict(torch::Tensor input);

    /**
     * @brief Use for training
     *
     * @param input
     * @return torch::Tensor
     */
    torch::Tensor forward(torch::Tensor input);
};

class SolutionDataset : public torch::data::Dataset<SolutionDataset> {

  private:
    std::vector<torch::Tensor> _inputs;
    std::vector<torch::Tensor> _utility_operator;
    size_t _size;

  public:
    SolutionDataset(const std::vector<torch::Tensor> &inputs_,
                    const std::vector<torch::Tensor> &utility_operator_);

    torch::data::Example<> get(size_t index) override;

    torch::optional<size_t> size() const override;
};

torch::Tensor solution_to_tensor(const Solution &solution);

template <typename Dataloader>
void train(Dataloader &data_loader,
           NeuralNetwork &model,
           torch::optim::Adam &optimizer,
           const int nb_epoch) {
    for (int epoch = 1; epoch <= nb_epoch; ++epoch) {
        // int batch_index = 0; // TODO comment
        // Iterate the data loader to yield batches from the dataset.
        for (auto &batch : *data_loader) {
            if (Parameters::p->time_limit_reached()) {
                continue;
            }
            // Reset gradients.
            optimizer.zero_grad();
            // Execute the model on the input data.
            // std::cout << "batch.data :\n" << batch.data.sizes();
            // std::cout << "batch.data :\n" << batch.data;

            torch::Tensor prediction = model.forward(batch.data);
            // Compute a loss value to judge the prediction of our model.
            // std::cout << "batch.target :\n" << batch.target.sizes() << "\n";
            // std::cout << "batch.target :\n" << batch.target << "\n";
            // std::cout << "prediction :\n" << prediction.sizes() << "\n";
            // std::cout << "prediction :\n" << prediction << "\n";

            auto real_target = torch::zeros_like(prediction);
            real_target.copy_(prediction);

            // std::cout << "prediction :\n" << prediction << "\n";

            auto actions = batch.target.index({torch::indexing::Slice(), 1});
            auto reward =
                batch.target.index({torch::indexing::Slice(), 0}).to(torch::kFloat);
            // std::cout << "actions :\n" << actions << "\n";
            // std::cout << "reward :\n" << reward << "\n";

            real_target =
                real_target.scatter_(1, actions.unsqueeze(1), reward.unsqueeze(1));

            // std::cout << "real_target :\n" << real_target << "\n";

            torch::Tensor loss = torch::mse_loss(prediction, real_target);

            // Compute gradients of the loss w.r.t. the parameters of our model.
            loss.backward();
            // Update the parameters based on the calculated gradients.
            optimizer.step();
            // Output the loss and checkpoint every 100 batches.
            // if (++batch_index % 100 == 0) {
            // fmt::print("Epoch: {:<3} | Batch: {:<3} | Loss: {:<10}\n",
            //            epoch,
            //            ++batch_index,
            //            loss.item<float>()); // TODO comment
            // Serialize your model periodically as a checkpoint.
            // torch::save(model, "net.pt");
            // }
        }
    }
}
